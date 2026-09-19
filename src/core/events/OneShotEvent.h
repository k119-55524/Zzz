#pragma once

#include <mutex>
#include <vector>
#include <memory>
#include <functional>
#include <optional>
#include <tuple>
#include <utility>

#include "core/utils/Defines.h"
#include "core/utils/Ensure.h"

namespace zzz::core
{
	/**
	 * @brief Низкоуровневый потокобезопасный примитив одноразового события (Pending -> Resolved).
	 *
	 * Позволяет нескольким потокам регистрировать подписчиков как до разрешения события, так и после.
	 * При поздней подписке (после перехода в Resolved) вызов колбэка безусловно маршалится
	 * через зарегистрированный диспетчер (например, в очередь главного потока m_MainThreadQueue),
	 * исключая дедлоки под мьютексами и гонки потоков.
	 */
	template<typename... Args>
	class OneShotEvent final
	{
		Z_NO_COPY_MOVE(OneShotEvent);

	public:
		using CallbackType = std::function<void(Args...)>;
		using DispatcherFunc = std::function<void(std::function<void()>)>;

		enum class State : uint8_t
		{
			Pending,
			Resolved
		};

	private:
		struct Subscriber
		{
			std::weak_ptr<void> context;
			bool hasContext = false;
			CallbackType callback;
		};

		mutable std::mutex m_Mutex;
		State m_State = State::Pending;
		std::optional<std::tuple<Args...>> m_Result;
		std::vector<Subscriber> m_Subscribers;
		DispatcherFunc m_Dispatcher;

	public:
		OneShotEvent() = delete;

		explicit OneShotEvent(DispatcherFunc dispatcher)
			: m_Dispatcher(std::move(dispatcher))
		{
			ensure(m_Dispatcher != nullptr, "OneShotEvent: dispatcher не установлен");
		}

		~OneShotEvent() = default;

		/// @brief Подписка с проверкой контекста подписчика
		template<typename ContextType>
		void Subscribe(std::weak_ptr<ContextType> context, CallbackType callback)
		{
			if (!callback)
				return;

			std::unique_lock lock(m_Mutex);
			if (m_State == State::Resolved)
			{
				auto res = *m_Result;
				auto dispatcher = m_Dispatcher;
				lock.unlock();
				DispatchSubscriber(Subscriber{ std::move(context), true, std::move(callback) }, res, dispatcher);
				return;
			}

			m_Subscribers.push_back(Subscriber{ std::move(context), true, std::move(callback) });
		}

		/// @brief Подписка без контекста (вызывается безусловно)
		void Subscribe(CallbackType callback)
		{
			if (!callback)
				return;

			std::unique_lock lock(m_Mutex);
			if (m_State == State::Resolved)
			{
				auto res = *m_Result;
				auto dispatcher = m_Dispatcher;
				lock.unlock();
				DispatchSubscriber(Subscriber{ {}, false, std::move(callback) }, res, dispatcher);
				return;
			}

			m_Subscribers.push_back(Subscriber{ {}, false, std::move(callback) });
		}

		/// @brief Однократный переход в состояние Resolved с доставкой результата всем подписчикам
		void Resolve(Args... args)
		{
			std::vector<Subscriber> subscribersToNotify;
			std::tuple<Args...> resultTuple(std::forward<Args>(args)...);
			DispatcherFunc dispatcher;

			{
				std::lock_guard lock(m_Mutex);
				if (m_State == State::Resolved)
				{
					// Однократное событие уже разрешено
					return;
				}

				m_State = State::Resolved;
				m_Result.emplace(resultTuple);
				subscribersToNotify = std::move(m_Subscribers);
				dispatcher = m_Dispatcher;
			}

			for (auto& sub : subscribersToNotify)
			{
				DispatchSubscriber(std::move(sub), resultTuple, dispatcher);
			}
		}

		/// @brief Проверка, перешло ли событие в состояние Resolved
		[[nodiscard]] bool IsResolved() const noexcept
		{
			std::lock_guard lock(m_Mutex);
			return m_State == State::Resolved;
		}

		/// @brief Получение сохраненного результата, если событие уже разрешено
		[[nodiscard]] std::optional<std::tuple<Args...>> GetResult() const
		{
			std::lock_guard lock(m_Mutex);
			return m_Result;
		}

	private:
		static void DispatchSubscriber(Subscriber sub, const std::tuple<Args...>& result, const DispatcherFunc& dispatcher)
		{
			auto task = [sub = std::move(sub), result]() mutable
			{
				std::shared_ptr<void> pinnedContext;
				if (sub.hasContext)
				{
					pinnedContext = sub.context.lock();
					if (!pinnedContext)
					{
						return;
					}
				}
				std::apply(sub.callback, result);
			};

			ensure(dispatcher != nullptr, "OneShotEvent: dispatcher не установлен");
			dispatcher(std::move(task));
		}
	};
}
