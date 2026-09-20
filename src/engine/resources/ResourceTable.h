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
	 * @details Инкапсулирует хэш-таблицу событий OneShotEvent<ResultType>, shared_mutex
	 *          и реактивную механику "подпишись на шот или запусти загрузку".
	 *          Соблюдает контракт Zero User Code Under Lock: замки таблицы всегда
	 *          освобождаются до вызова внешних функций (Subscribe, onLoadRequest, Resolve).
	 */
	template<typename T>
	class ResourceTable final
	{
		Z_NO_COPY_MOVE(ResourceTable);

	public:
		using ResultType = std::expected<std::shared_ptr<T>, std::string>;
		using CallbackType = std::function<void(ResultType)>;
		using CallbackDispatcher = std::function<void(std::function<void()>)>;
		using EventType = OneShotEvent<ResultType>;

		ResourceTable() = delete;
		explicit ResourceTable(CallbackDispatcher dispatcher)
			: m_CallbackDispatcher(std::move(dispatcher))
		{
			ensure(m_CallbackDispatcher != nullptr, "ResourceTable: CallbackDispatcher не должен быть null");
		}

		~ResourceTable() = default;

		/**
		 * @brief Асинхронный запрос ресурса по GUID с контекстом жизни (weak_ptr).
		 * @details Реализует Read-First паттерн: сначала поиск под shared_lock,
		 *          при отсутствии — вставка под unique_lock.
		 *          Замки снимаются перед подпиской и запуском onLoadRequest.
		 */
		template<typename ContextType, typename LoadFunc>
		void GetOrRequest(
			const Guid& guid,
			std::weak_ptr<ContextType> context,
			CallbackType onLoaded,
			LoadFunc&& onLoadRequest)
		{
			std::shared_ptr<EventType> eventPtr;
			bool isNew = false;

			// Фаза 1: Read-First под shared_lock
			{
				std::shared_lock readLock(m_Mutex);
				auto it = m_Events.find(guid);
				if (it != m_Events.end())
				{
					eventPtr = it->second;
				}
			}

			// Фаза 2: Если не найден — переходим под unique_lock
			if (!eventPtr)
			{
				std::unique_lock writeLock(m_Mutex);
				auto it = m_Events.find(guid);
				if (it != m_Events.end())
				{
					eventPtr = it->second;
				}
				else
				{
					eventPtr = std::make_shared<EventType>(m_CallbackDispatcher);
					m_Events.emplace(guid, eventPtr);
					isNew = true;
				}
			}

			// Фаза 3: Zero User Code Under Lock
			ensure(eventPtr != nullptr, "ResourceTable: eventPtr не должен быть null");
			eventPtr->Subscribe(std::move(context), std::move(onLoaded));

			if (isNew)
			{
				onLoadRequest(guid);
			}
		}

		/**
		 * @brief Асинхронный запрос ресурса по GUID без контекста жизни.
		 */
		template<typename LoadFunc>
		void GetOrRequest(
			const Guid& guid,
			CallbackType onLoaded,
			LoadFunc&& onLoadRequest)
		{
			std::shared_ptr<EventType> eventPtr;
			bool isNew = false;

			// Фаза 1: Read-First под shared_lock
			{
				std::shared_lock readLock(m_Mutex);
				auto it = m_Events.find(guid);
				if (it != m_Events.end())
				{
					eventPtr = it->second;
				}
			}

			// Фаза 2: Если не найден — переходим под unique_lock
			if (!eventPtr)
			{
				std::unique_lock writeLock(m_Mutex);
				auto it = m_Events.find(guid);
				if (it != m_Events.end())
				{
					eventPtr = it->second;
				}
				else
				{
					eventPtr = std::make_shared<EventType>(m_CallbackDispatcher);
					m_Events.emplace(guid, eventPtr);
					isNew = true;
				}
			}

			// Фаза 3: Zero User Code Under Lock
			ensure(eventPtr != nullptr, "ResourceTable: eventPtr не должен быть null");
			eventPtr->Subscribe(std::move(onLoaded));

			if (isNew)
			{
				onLoadRequest(guid);
			}
		}

		/**
		 * @brief Разрешение события готовности (успех или ошибка).
		 * @details Поиск выполняется под shared_lock строго без создания новой записи.
		 *          Замок снимается до вызова eventPtr->Resolve.
		 */
		bool Resolve(const Guid& guid, ResultType result)
		{
			std::shared_ptr<EventType> eventPtr;

			{
				std::shared_lock lock(m_Mutex);
				auto it = m_Events.find(guid);
				if (it != m_Events.end())
				{
					eventPtr = it->second;
				}
			}

			if (!eventPtr)
				return false;

			eventPtr->Resolve(std::move(result));
			return true;
		}

		/// @brief Синхронная попытка получить готовый ресурс из таблицы без ожидания
		[[nodiscard]] std::shared_ptr<T> TryGet(const Guid& guid) const
		{
			std::shared_ptr<EventType> eventPtr;
			{
				std::shared_lock lock(m_Mutex);
				auto it = m_Events.find(guid);
				if (it != m_Events.end())
				{
					eventPtr = it->second;
				}
			}

			if (eventPtr)
			{
				auto resOpt = eventPtr->GetResult();
				if (resOpt)
				{
					const auto& expectedRes = std::get<0>(*resOpt);
					if (expectedRes)
					{
						return *expectedRes;
					}
				}
			}
			return nullptr;
		}

		/// @brief Проверка наличия записи в таблице по GUID
		[[nodiscard]] bool Contains(const Guid& guid) const
		{
			std::shared_lock lock(m_Mutex);
			return m_Events.contains(guid);
		}

		/// @brief Очистка всех записей таблицы
		void Clear()
		{
			std::unique_lock lock(m_Mutex);
			m_Events.clear();
		}

		[[nodiscard]] size_t Size() const noexcept
		{
			std::shared_lock lock(m_Mutex);
			return m_Events.size();
		}

	private:
		mutable std::shared_mutex m_Mutex;
		std::unordered_map<Guid, std::shared_ptr<EventType>> m_Events;
		CallbackDispatcher m_CallbackDispatcher;
	};
}
