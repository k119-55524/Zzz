#pragma once

#include <memory>
#include <string>
#include <expected>
#include <functional>
#include <shared_mutex>
#include <unordered_map>

#include "core/utils/Guid.h"
#include "core/events/OneShotEvent.h"
#include "core/utils/macros/MiscMacros.h"

using namespace zzz::core;

namespace zzz::engine
{
	/**
	 * @class ResourceTable
	 * @brief Потокобезопасная типизированная таблица ресурсов конкретного типа T.
	 * @details Хранит слабые ссылки std::weak_ptr на загруженные ресурсы и OneShotEvent
	 *          для ожидающих in-flight загрузок. Ресурсы в памяти удерживаются внешними
	 *          ResourceRef; при обнулении внешних ссылок ресурс автоматически удаляется из RAM/VRAM.
	 *          Соблюдает контракт Zero User Code Under Lock: замки таблицы всегда
	 *          освобождаются до вызова внешних функций (onLoaded, onLoadRequest, Resolve).
	 */
	template<typename T>
	class ResourceTable final
	{
		Z_NO_COPY_MOVE(ResourceTable);

	public:
		using ResultType = std::expected<std::shared_ptr<T>, std::string>;
		using CallbackType = std::function<void(ResultType)>;
		using ResourceEvent = OneShotEvent<ResultType>;

		ResourceTable() = default;
		~ResourceTable() = default;

		/**
		 * @brief Асинхронный запрос ресурса по GUID с контекстом жизни (weak_ptr).
		 * @details Реализует Read-First паттерн:
		 *          1. Поиск живого ресурса в слабом кэше (weak_ptr.lock()). При попадании — немедленный вызов колбэка.
		 *          2. Если ресурс уже загружается другим запросом — подписка на in-flight событие.
		 *          3. Если ресурса нет или он выгружен — создание нового in-flight события и вызов onLoadRequest.
		 *          Замки таблицы всегда снимаются до вызова onLoaded и onLoadRequest.
		 */
		template<typename ContextType, typename LoadFunc>
		void GetOrRequest(
			const Guid& guid,
			std::weak_ptr<ContextType> context,
			CallbackType onLoaded,
			LoadFunc&& onLoadRequest)
		{
			std::shared_ptr<T> cachedResource;
			std::shared_ptr<ResourceEvent> inFlight;
			bool isNew = false;

			// Read-lock: быстрая проверка кэша и in-flight
			{
				std::shared_lock readLock(m_Mutex);

				auto it = m_Resources.find(guid);
				if (it != m_Resources.end())
				{
					cachedResource = it->second.weakResource.lock();
					if (!cachedResource)
					{
						inFlight = it->second.inFlightEvent;
					}
				}
			}

			if (cachedResource)
			{
				if (auto ctx = context.lock())
				{
					onLoaded(std::move(cachedResource));
				}
				return;
			}

			if (inFlight)
			{
				inFlight->Subscribe(std::move(context), std::move(onLoaded));
				return;
			}

			// Write-lock: создание in-flight записи при отсутствии
			{
				std::unique_lock writeLock(m_Mutex);

				auto& slot = m_Resources[guid];
				cachedResource = slot.weakResource.lock();
				if (cachedResource)
				{
					// Успел загрузиться в параллельном потоке
				}
				else if (slot.inFlightEvent)
				{
					inFlight = slot.inFlightEvent;
				}
				else
				{
					inFlight = std::make_shared<ResourceEvent>();
					slot.inFlightEvent = inFlight;
					isNew = true;
				}
			}

			if (cachedResource)
			{
				if (auto ctx = context.lock())
				{
					onLoaded(std::move(cachedResource));
				}
				return;
			}

			inFlight->Subscribe(std::move(context), std::move(onLoaded));
			if (isNew)
			{
				onLoadRequest(guid);
			}
		}

		/**
		 * @brief Разрешение записи ресурса (успех или ошибка).
		 * @details При успехе сохраняет слабую ссылку std::weak_ptr в слот, оповещает
		 *          подписчиков и сбрасывает in-flight событие. Таблица НЕ удерживает
		 *          жесткий shared_ptr. Замок снимается до вызова Resolve.
		 */
		bool Resolve(const Guid& guid, ResultType result)
		{
			std::shared_ptr<ResourceEvent> eventToResolve;

			{
				std::unique_lock lock(m_Mutex);
				auto it = m_Resources.find(guid);
				if (it == m_Resources.end())
					return false;

				eventToResolve = std::move(it->second.inFlightEvent);
				it->second.inFlightEvent.reset();

				if (result.has_value())
				{
					it->second.weakResource = *result;
				}
				else
				{
					if (it->second.weakResource.expired())
					{
						m_Resources.erase(it);
					}
				}
			}

			if (!eventToResolve)
				return false;

			eventToResolve->Resolve(std::move(result));
			return true;
		}

		/// @brief Синхронная попытка получить готовый ресурс из таблицы без ожидания
		[[nodiscard]] std::shared_ptr<T> TryGet(const Guid& guid) const
		{
			std::shared_lock lock(m_Mutex);
			auto it = m_Resources.find(guid);
			if (it != m_Resources.end())
			{
				return it->second.weakResource.lock();
			}
			return nullptr;
		}

		/// @brief Очистка всех записей таблицы
		void Clear()
		{
			std::unique_lock lock(m_Mutex);
			m_Resources.clear();
		}

	private:
		struct ResourceSlot
		{
			std::weak_ptr<T> weakResource;
			std::shared_ptr<ResourceEvent> inFlightEvent;
		};

		mutable std::shared_mutex m_Mutex;
		std::unordered_map<Guid, ResourceSlot> m_Resources;
	};
}
