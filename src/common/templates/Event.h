#pragma once

#include <mutex>
#include <vector>
#include <memory>
#include <atomic>
#include <functional>
#include <common/common.h>
#include <common/throw_wrappers.h>

namespace zzz::engine
{
	template<typename FuncType>
	struct CallbackEntry
	{
		FuncType func;
		std::weak_ptr<void> context;
		std::atomic<bool> isDead = false;

		CallbackEntry() = default;

		CallbackEntry(FuncType f, std::weak_ptr<void> ctx, bool dead = false)
			: func(std::move(f)), context(std::move(ctx)), isDead(dead) {}

		CallbackEntry(const CallbackEntry& other)
			: func(other.func), context(other.context), isDead(other.isDead.load()) {}

		CallbackEntry(CallbackEntry&& other) noexcept
			: func(std::move(other.func)), context(std::move(other.context)), isDead(other.isDead.load()) {}

		CallbackEntry& operator=(const CallbackEntry& other)
		{
			func = other.func;
			context = other.context;
			isDead.store(other.isDead.load());
			return *this;
		}

		CallbackEntry& operator=(CallbackEntry&& other) noexcept
		{
			func = std::move(other.func);
			context = std::move(other.context);
			isDead.store(other.isDead.load());
			return *this;
		}
	};

	template<typename FuncType, bool Ordered>
	class EventBase
	{
	protected:
		using EntryType = CallbackEntry<FuncType>;
		using CallbackList = std::vector<EntryType>;

		CallbackList listeners;
		CallbackList pendingAdditions;

		std::mutex eventMutex;
		std::vector<std::thread::id> invokingThreads;
		std::atomic<bool> needsCleanup = false;
		std::atomic<bool*> destroyFlag = nullptr;

	public:
		EventBase() = default;

		EventBase(const EventBase&) = delete;
		EventBase& operator=(const EventBase&) = delete;
		EventBase(EventBase&&) = delete;
		EventBase& operator=(EventBase&&) = delete;

	protected:
		~EventBase()
		{
			bool* flag = destroyFlag.load();
			if (flag)
				*flag = true;
		}

		void CheckRecursion(std::thread::id thisThread)
		{
			if (std::find(invokingThreads.begin(), invokingThreads.end(), thisThread) != invokingThreads.end())
				THROW_RUNTIME("[FATAL ERROR] Recursive invocation detected!");
		}

		void CheckModifying()
		{
			if (!invokingThreads.empty())
				THROW_RUNTIME("[FATAL ERROR] Cannot modify Event while invoking!");
		}

		struct InvocationGuard
		{
			EventBase* event;
			bool* destroyed;
			std::thread::id thisThread;

			explicit InvocationGuard(EventBase* e, bool* d)
				: event(e), destroyed(d), thisThread(std::this_thread::get_id())
			{
				std::lock_guard<std::mutex> lock(event->eventMutex);
				event->CheckRecursion(thisThread);

				event->destroyFlag.store(destroyed);
				event->invokingThreads.push_back(thisThread);
			}

			~InvocationGuard()
			{
				if (*destroyed)
					return;

				std::lock_guard<std::mutex> lock(event->eventMutex);

				auto itThread = std::find(event->invokingThreads.begin(), event->invokingThreads.end(), thisThread);
				if (itThread != event->invokingThreads.end())
					event->invokingThreads.erase(itThread);

				if (event->invokingThreads.empty())
				{
					event->destroyFlag.store(nullptr);

					if (event->needsCleanup.load())
					{
						if constexpr (Ordered)
						{
							auto it = std::remove_if(event->listeners.begin(), event->listeners.end(), [](const EntryType& e) {
								return e.isDead.load() || e.context.expired();
							});
							event->listeners.erase(it, event->listeners.end());
						}
						else
						{
							for (size_t i = 0; i < event->listeners.size(); )
							{
								if (event->listeners[i].isDead.load() || event->listeners[i].context.expired())
								{
									if (i != event->listeners.size() - 1)
										event->listeners[i] = std::move(event->listeners.back());
									event->listeners.pop_back();
								}
								else
								{
									++i;
								}
							}
						}
						event->needsCleanup.store(false);
					}

					if (!event->pendingAdditions.empty())
					{
						event->listeners.insert(
							event->listeners.end(),
							std::make_move_iterator(event->pendingAdditions.begin()),
							std::make_move_iterator(event->pendingAdditions.end())
						);
						event->pendingAdditions.clear();
					}
				}
			}

			InvocationGuard(const InvocationGuard&) = delete;
			InvocationGuard& operator=(const InvocationGuard&) = delete;
		};
	};

	template<bool Ordered, typename... Args>
	class EventImpl final : public EventBase<std::function<void(Args...)>, Ordered>
	{
	public:
		using FuncType = std::function<void(Args...)>;
		using EntryType = CallbackEntry<FuncType>;

		EventImpl() = default;

		void Clear()
		{
			std::lock_guard<std::mutex> lock(this->eventMutex);
			this->CheckModifying();
			this->listeners.clear();
		}

		void Reserve(size_t count)
		{
			std::lock_guard<std::mutex> lock(this->eventMutex);
			this->CheckModifying();
			this->listeners.reserve(count);
		}

		template<typename ContextType>
		void Unsubscribe(std::shared_ptr<ContextType> context)
		{
			if (!context)
				return;

			std::lock_guard<std::mutex> lock(this->eventMutex);
			
			// 1. Ищем в основном списке
			for (auto& entry : this->listeners)
			{
				if (entry.context.lock() == context)
				{
					entry.isDead.store(true);
					this->needsCleanup.store(true);

					if (this->invokingThreads.empty())
					{
						entry.func = nullptr;
					}
				}
			}

			// 2. Ищем и сразу удаляем из очереди ожидающих, если они еще не добавлены
			if (!this->pendingAdditions.empty())
			{
				auto it = std::remove_if(this->pendingAdditions.begin(), this->pendingAdditions.end(),
					[&](const EntryType& e) {
						return e.context.lock() == context;
					});
				this->pendingAdditions.erase(it, this->pendingAdditions.end());
			}
		}

		template<typename ContextType, typename F>
		void Subscribe(std::weak_ptr<ContextType> context, F&& func)
		{
			FuncType f(std::forward<F>(func));
			if (!f)
				THROW_RUNTIME("[FATAL ERROR] Cannot subscribe an empty callback to Event!");

			std::lock_guard<std::mutex> lock(this->eventMutex);
			if (!this->invokingThreads.empty())
				this->pendingAdditions.push_back({ std::move(f), context, false });
			else
				this->listeners.push_back({ std::move(f), context, false });
		}

		template<typename ContextType, typename F>
		void Subscribe(std::shared_ptr<ContextType> context, F&& func)
		{
			Subscribe(std::weak_ptr<ContextType>(context), std::forward<F>(func));
		}

		/// @brief Вызывает всех подписчиков события.
		/// @note В зависимости от типа события, порядок может не сохраняться. Мертвые подписчики удаляются в конце.
		/// @param args Аргументы, которые будут переданы всем слушателям.
		void operator()(Args... args)
		{
			bool destroyed = false;
			typename Base::InvocationGuard guard(this, &destroyed);

			for (size_t i = 0; i < this->listeners.size(); ++i)
			{
				auto& entry = this->listeners[i];

				if (entry.isDead.load())
					continue;

				if (entry.context.expired())
				{
					entry.isDead.store(true);
					this->needsCleanup.store(true);
					continue;
				}

				auto func = entry.func;
				func(args...);

				if (destroyed)
					return;
			}
		}

	private:
		using Base = EventBase<std::function<void(Args...)>, Ordered>;
	};

	template<typename... Args>
	using Event = EventImpl<true, Args...>;

	template<typename... Args>
	using UnorderedEvent = EventImpl<false, Args...>;
}
