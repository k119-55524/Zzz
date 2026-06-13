#pragma once

#include <mutex>
#include <memory>
#include <vector>
#include <algorithm>
#include <functional>
#include <typeinfo>

namespace zzz::engine
{
	template<typename FuncType>
	struct ThreadSafeCallbackEntry
	{
		FuncType func;
		std::weak_ptr<void> context;
		const char* subscriberName;
		bool hasContext;
	};

	template<typename FuncType>
	class eventThreadSafeBase
	{
	protected:
		using EntryType = ThreadSafeCallbackEntry<FuncType>;
		using CallbackList = std::vector<EntryType>;
		std::shared_ptr<CallbackList> listeners;
		std::mutex listenersMutex;
		const char* ownerName = "Unknown";

	public:
		eventThreadSafeBase() = default;

		template<typename OwnerType>
		explicit eventThreadSafeBase(OwnerType* owner)
		{
			if (owner)
			{
				ownerName = typeid(*owner).name();
			}
		}

		void clear()
		{
			std::lock_guard<std::mutex> lock(listenersMutex);
			if (listeners)
			{
				listeners.reset();
			}
		}
	};

	template<typename... Args>
	class EventThreadSafe : public eventThreadSafeBase<std::function<void(Args...)>>
	{
	public:
		using FuncType = std::function<void(Args...)>;
		using EntryType = ThreadSafeCallbackEntry<FuncType>;
		using CallbackList = std::vector<EntryType>;

		EventThreadSafe() = default;

		template<typename OwnerType>
		explicit EventThreadSafe(OwnerType* owner) : eventThreadSafeBase<FuncType>(owner) {}

		// Потокобезопасная подписка статической функции (без контекста)
		template<typename F>
		void SubscribeStaticSafe(F&& func)
		{
			std::lock_guard<std::mutex> lock(this->listenersMutex);
			auto newListeners = std::make_shared<CallbackList>(this->listeners ? *this->listeners : CallbackList{});
			newListeners->push_back({ std::forward<F>(func), std::weak_ptr<void>(), "Static / No Context", false });
			this->listeners = newListeners;
		}

		// Потокобезопасная подписка функции с контекстом
		template<typename ContextType, typename F>
		void SubscribeSafe(std::weak_ptr<ContextType> context, F&& func)
		{
			std::lock_guard<std::mutex> lock(this->listenersMutex);
			auto newListeners = std::make_shared<CallbackList>(this->listeners ? *this->listeners : CallbackList{});
			newListeners->push_back({ std::forward<F>(func), context, typeid(ContextType).name(), true });
			this->listeners = newListeners;
		}

		template<typename ContextType, typename F>
		void SubscribeSafe(std::shared_ptr<ContextType> context, F&& func)
		{
			std::lock_guard<std::mutex> lock(this->listenersMutex);
			auto newListeners = std::make_shared<CallbackList>(this->listeners ? *this->listeners : CallbackList{});
			newListeners->push_back({ std::forward<F>(func), context, typeid(ContextType).name(), true });
			this->listeners = newListeners;
		}

		void operator()(Args... args)
		{
			std::shared_ptr<CallbackList> currentListeners;
			{
				std::lock_guard<std::mutex> lock(this->listenersMutex);
				currentListeners = this->listeners;
			}

			if (!currentListeners)
				return;

			bool needsCleanup = false;

			for (auto& cb : *currentListeners)
			{
				if (cb.hasContext)
				{
					if (cb.context.expired())
					{
						needsCleanup = true;
					}
					else
					{
						cb.func(args...);
					}
				}
				else
				{
					cb.func(args...);
				}
			}

			if (needsCleanup)
			{
				Cleanup();
			}
		}

	private:
		void Cleanup()
		{
			std::lock_guard<std::mutex> lock(this->listenersMutex);
			if (!this->listeners)
				return;

			auto newListeners = std::make_shared<CallbackList>();
			newListeners->reserve(this->listeners->size());

			for (auto& cb : *this->listeners)
			{
				if (!cb.hasContext || !cb.context.expired())
				{
					newListeners->push_back(cb);
				}
			}

			if (newListeners->empty())
				this->listeners.reset();
			else
				this->listeners = newListeners;
		}
	};

	template<>
	class EventThreadSafe<void> : public eventThreadSafeBase<std::function<void()>>
	{
	public:
		using FuncType = std::function<void()>;
		using EntryType = ThreadSafeCallbackEntry<FuncType>;
		using CallbackList = std::vector<EntryType>;

		EventThreadSafe() = default;

		template<typename OwnerType>
		explicit EventThreadSafe(OwnerType* owner) : eventThreadSafeBase<FuncType>(owner) {}

		template<typename F>
		void SubscribeStaticSafe(F&& func)
		{
			std::lock_guard<std::mutex> lock(this->listenersMutex);
			auto newListeners = std::make_shared<CallbackList>(this->listeners ? *this->listeners : CallbackList{});
			newListeners->push_back({ std::forward<F>(func), std::weak_ptr<void>(), "Static / No Context", false });
			this->listeners = newListeners;
		}

		template<typename ContextType, typename F>
		void SubscribeSafe(std::weak_ptr<ContextType> context, F&& func)
		{
			std::lock_guard<std::mutex> lock(this->listenersMutex);
			auto newListeners = std::make_shared<CallbackList>(this->listeners ? *this->listeners : CallbackList{});
			newListeners->push_back({ std::forward<F>(func), context, typeid(ContextType).name(), true });
			this->listeners = newListeners;
		}

		template<typename ContextType, typename F>
		void SubscribeSafe(std::shared_ptr<ContextType> context, F&& func)
		{
			std::lock_guard<std::mutex> lock(this->listenersMutex);
			auto newListeners = std::make_shared<CallbackList>(this->listeners ? *this->listeners : CallbackList{});
			newListeners->push_back({ std::forward<F>(func), context, typeid(ContextType).name(), true });
			this->listeners = newListeners;
		}

		void operator()()
		{
			std::shared_ptr<CallbackList> currentListeners;
			{
				std::lock_guard<std::mutex> lock(this->listenersMutex);
				currentListeners = this->listeners;
			}

			if (!currentListeners)
				return;

			bool needsCleanup = false;

			for (auto& cb : *currentListeners)
			{
				if (cb.hasContext)
				{
					if (cb.context.expired())
					{
						needsCleanup = true;
					}
					else
					{
						cb.func();
					}
				}
				else
				{
					cb.func();
				}
			}

			if (needsCleanup)
			{
				Cleanup();
			}
		}

	private:
		void Cleanup()
		{
			std::lock_guard<std::mutex> lock(this->listenersMutex);
			if (!this->listeners)
				return;

			auto newListeners = std::make_shared<CallbackList>();
			newListeners->reserve(this->listeners->size());

			for (auto& cb : *this->listeners)
			{
				if (!cb.hasContext || !cb.context.expired())
				{
					newListeners->push_back(cb);
				}
			}

			if (newListeners->empty())
				this->listeners.reset();
			else
				this->listeners = newListeners;
		}
	};
}
