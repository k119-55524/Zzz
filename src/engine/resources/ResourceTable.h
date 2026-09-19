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
#include "engine/resources/ResourceRecord.h"

using namespace zzz::core;

namespace zzz::engine
{
	/**
	 * @class ResourceTable
	 * @brief Потокобезопасная типизированная таблица ресурсов конкретного типа T.
	 * @details Инкапсулирует хэш-таблицу записей ResourceRecord<T>, shared_mutex
	 *          и реактивную механику "верни из кэша или поставь на загрузку".
	 */
	template<typename T>
	class ResourceTable final
	{
		Z_NO_COPY_MOVE(ResourceTable);

	public:
		using ResultType = std::expected<std::shared_ptr<T>, std::string>;
		using CallbackType = std::function<void(ResultType)>;
		using DispatcherFunc = typename OneShotEvent<ResultType>::DispatcherFunc;

		ResourceTable() = default;
		~ResourceTable() = default;

		/**
		 * @brief Асинхронный запрос ресурса по GUID с контекстом жизни (weak_ptr).
		 * @details Если запись уже есть — атомарно подписывает коллбэк (вызовется сразу, если ресурс уже готов).
		 *          Если записи нет — атомарно создаёт запись, подписывает коллбэк и вызывает onLoadRequest(guid).
		 */
		template<typename ContextType, typename LoadFunc>
		void GetOrRequest(
			const Guid& guid,
			std::weak_ptr<ContextType> context,
			CallbackType onLoaded,
			const DispatcherFunc& dispatcher,
			LoadFunc&& onLoadRequest)
		{
			std::unique_lock lock(m_Mutex);
			auto [it, inserted] = m_Records.try_emplace(guid, dispatcher);
			it->second.readyEvent.Subscribe(std::move(context), std::move(onLoaded));
			if (inserted)
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
			const DispatcherFunc& dispatcher,
			LoadFunc&& onLoadRequest)
		{
			std::unique_lock lock(m_Mutex);
			auto [it, inserted] = m_Records.try_emplace(guid, dispatcher);
			it->second.readyEvent.Subscribe(std::move(onLoaded));
			if (inserted)
			{
				onLoadRequest(guid);
			}
		}

		/// @brief Разрешение события готовности (успех или ошибка)
		void Resolve(const Guid& guid, ResultType result, const DispatcherFunc& dispatcher)
		{
			std::unique_lock lock(m_Mutex);
			auto [it, _] = m_Records.try_emplace(guid, dispatcher);
			if (result)
			{
				it->second.resource = *result;
			}
			it->second.readyEvent.Resolve(std::move(result));
		}

		/// @brief Очистка всех записей таблицы
		void Clear()
		{
			std::unique_lock lock(m_Mutex);
			m_Records.clear();
		}

		[[nodiscard]] size_t Size() const noexcept
		{
			std::shared_lock lock(m_Mutex);
			return m_Records.size();
		}

	private:
		mutable std::shared_mutex m_Mutex;
		std::unordered_map<Guid, ResourceRecord<T>> m_Records;
	};
}
