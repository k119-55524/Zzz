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
		 * @details Если шот уже есть — атомарно подписывает коллбэк (вызовется сразу, если ресурс готов).
		 *          Если шота нет — атомарно создаёт шот, подписывает коллбэк и вызывает onLoadRequest(guid).
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

			auto [it, inserted] = m_Events.try_emplace(guid, dispatcher);
			it->second.Subscribe(std::move(context), std::move(onLoaded));

			if (inserted)
				onLoadRequest(guid);
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

			auto [it, inserted] = m_Events.try_emplace(guid, dispatcher);
			it->second.Subscribe(std::move(onLoaded));

			if (inserted)
				onLoadRequest(guid);
		}

		/// @brief Разрешение события готовности (успех или ошибка)
		void Resolve(const Guid& guid, ResultType result, const DispatcherFunc& dispatcher)
		{
			std::unique_lock lock(m_Mutex);
			auto [it, _] = m_Events.try_emplace(guid, dispatcher);
			it->second.Resolve(std::move(result));
		}

		/// @brief Синхронная попытка получить готовый ресурс из таблицы без ожидания
		[[nodiscard]] std::shared_ptr<T> TryGet(const Guid& guid) const
		{
			std::shared_lock lock(m_Mutex);
			auto it = m_Events.find(guid);
			if (it != m_Events.end())
			{
				auto resOpt = it->second.GetResult();
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
			m_Events.clear();
		}

		[[nodiscard]] size_t Size() const noexcept
		{
			std::shared_lock lock(m_Mutex);
			return m_Events.size();
		}

	private:
		mutable std::shared_mutex m_Mutex;
		std::unordered_map<Guid, OneShotEvent<ResultType>> m_Events;
	};
}
