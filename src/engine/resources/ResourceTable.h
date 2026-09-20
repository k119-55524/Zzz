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
		using ResourceEntry = OneShotEvent<ResultType>;

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
			std::shared_ptr<ResourceEntry> entry;
			bool isNew = false;

			// Фаза 1: Read-First под shared_lock
			{
				std::shared_lock readLock(m_Mutex);
				auto it = m_Resources.find(guid);
				if (it != m_Resources.end())
				{
					entry = it->second;
				}
			}

			// Фаза 2: Если не найден — переходим под unique_lock
			if (!entry)
			{
				std::unique_lock writeLock(m_Mutex);
				auto it = m_Resources.find(guid);
				if (it != m_Resources.end())
				{
					entry = it->second;
				}
				else
				{
					entry = std::make_shared<ResourceEntry>(m_CallbackDispatcher);
					m_Resources.emplace(guid, entry);
					isNew = true;
				}
			}

			// Фаза 3: Zero User Code Under Lock
			ensure(entry != nullptr, "ResourceTable: entry не должен быть null");
			entry->Subscribe(std::move(context), std::move(onLoaded));

			if (isNew)
			{
				onLoadRequest(guid);
			}
		}

		/**
		 * @brief Разрешение записи ресурса (успех или ошибка).
		 * @details Поиск выполняется под shared_lock строго без создания новой записи.
		 *          Замок снимается до вызова entry->Resolve.
		 */
		bool Resolve(const Guid& guid, ResultType result)
		{
			std::shared_ptr<ResourceEntry> entry;

			{
				std::shared_lock lock(m_Mutex);
				auto it = m_Resources.find(guid);
				if (it != m_Resources.end())
				{
					entry = it->second;
				}
			}

			if (!entry)
				return false;

			entry->Resolve(std::move(result));
			return true;
		}

		/// @brief Синхронная попытка получить готовый ресурс из таблицы без ожидания
		[[nodiscard]] std::shared_ptr<T> TryGet(const Guid& guid) const
		{
			std::shared_ptr<ResourceEntry> entry;
			{
				std::shared_lock lock(m_Mutex);
				auto it = m_Resources.find(guid);
				if (it != m_Resources.end())
				{
					entry = it->second;
				}
			}

			if (entry)
			{
				auto resOpt = entry->GetResult();
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

		/// @brief Очистка всех записей таблицы
		void Clear()
		{
			std::unique_lock lock(m_Mutex);
			m_Resources.clear();
		}

	private:
		mutable std::shared_mutex m_Mutex;
		std::unordered_map<Guid, std::shared_ptr<ResourceEntry>> m_Resources;
		CallbackDispatcher m_CallbackDispatcher;
	};
}
