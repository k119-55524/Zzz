#pragma once

#include <tuple>
#include <mutex>
#include <vector>
#include <memory>
#include <utility>
#include <optional>
#include <functional>

#include "core/utils/Ensure.h"
#include "core/utils/macros/MiscMacros.h"

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
			ensure(callback != nullptr, "Callback не должен быть null");

			std::unique_lock lock(m_Mutex);
			if (m_State == State::Resolved)
			{
				auto res = *m_Result;
				lock.unlock();
				DispatchSubscriber(Subscriber{ std::move(context), true, std::move(callback) }, res);

				return;
			}

			m_Subscribers.push_back(Subscriber{ std::move(context), true, std::move(callback) });
		}

		/// @brief Подписка без контекста (вызывается безусловно)
		void Subscribe(CallbackType callback)
		{
			ensure(callback != nullptr, "Callback не должен быть null");

			std::unique_lock lock(m_Mutex);
			if (m_State == State::Resolved)
			{
				auto res = *m_Result;
				lock.unlock();
				DispatchSubscriber(Subscriber{ {}, false, std::move(callback) }, res);

				return;
			}

			m_Subscribers.push_back(Subscriber{ {}, false, std::move(callback) });
		}

		/// @brief Однократный переход в состояние Resolved с доставкой результата всем подписчикам
		void Resolve(Args... args)
		{
			std::vector<Subscriber> subscribersToNotify;
			std::tuple<Args...> resultTuple(std::forward<Args>(args)...);

			{
				std::lock_guard lock(m_Mutex);

				// Однократное событие уже разрешено
				if (m_State == State::Resolved)
					return;

				m_State = State::Resolved;
				m_Result.emplace(resultTuple);
				subscribersToNotify = std::move(m_Subscribers);
			}

			for (auto& sub : subscribersToNotify)
				DispatchSubscriber(std::move(sub), resultTuple);
		}

		/// @brief Проверка, перешло ли событие в состояние Resolved
		[[nodiscard]] bool IsResolved() const noexcept
		{
			std::lock_guard lock(m_Mutex);
			return m_State == State::Resolved;
		}

		/// @brief Получение сохранённого результата события (если IsResolved() == true)
		[[nodiscard]] std::optional<std::tuple<Args...>> GetResult() const
		{
			std::lock_guard lock(m_Mutex);
			return m_Result;
		}

	private:
		void DispatchSubscriber(Subscriber inSub, const std::tuple<Args...>& result)
		{
			auto task = [sub = std::move(inSub), result]() mutable
			{
				std::shared_ptr<void> pinnedContext;
				if (sub.hasContext)
				{
					pinnedContext = sub.context.lock();
					if (!pinnedContext)
						return;
				}

				std::apply(sub.callback, result);
			};

			ensure(m_Dispatcher != nullptr, "OneShotEvent: dispatcher не установлен");
			m_Dispatcher(std::move(task));
		}
	};
}
