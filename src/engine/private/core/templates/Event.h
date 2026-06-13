#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <functional>
#include <typeinfo>
#include <iostream>
#include <cstdlib>

namespace zzz::engine
{
	template<typename FuncType>
	struct CallbackEntry
	{
		FuncType func;
		std::weak_ptr<void> context;
		const char* subscriberName;
		bool hasContext;
	};

	template<typename FuncType>
	class eventBase
	{
	protected:
		using EntryType = CallbackEntry<FuncType>;
		using CallbackList = std::vector<EntryType>;
		CallbackList listeners;
		const char* ownerName = "Unknown";
		const char* executingSubscriber = nullptr;
		bool isInvoking = false;

	public:
		eventBase() = default;

		template<typename OwnerType>
		explicit eventBase(OwnerType* owner)
		{
			if (owner)
			{
				ownerName = typeid(*owner).name();
			}
		}

		void clear()
		{
			listeners.clear();
		}

	protected:
		void CheckRecursion()
		{
			if (isInvoking)
			{
				std::cerr << "[FATAL ERROR] Recursive event invocation detected!\n"
					<< "Event Owner: " << ownerName << "\n"
					<< "Executing Subscriber: " << (executingSubscriber ? executingSubscriber : "Unknown") << "\n";
				std::abort();
			}
		}
	};

	template<typename... Args>
	class Event : public eventBase<std::function<void(Args...)>>
	{
	public:
		using FuncType = std::function<void(Args...)>;
		using EntryType = CallbackEntry<FuncType>;
		using CallbackList = std::vector<EntryType>;

		Event() = default;

		template<typename OwnerType>
		explicit Event(OwnerType* owner) : eventBase<FuncType>(owner) {}

		// НЕ потокобезопасная подписка статической функции
		template<typename F>
		void SubscribeStaticUnsafe(F&& func)
		{
			this->listeners.push_back({ std::forward<F>(func), std::weak_ptr<void>(), "Static / No Context", false });
		}

		// НЕ потокобезопасная подписка функции с контекстом
		template<typename ContextType, typename F>
		void SubscribeUnsafe(std::weak_ptr<ContextType> context, F&& func)
		{
			this->listeners.push_back({ std::forward<F>(func), context, typeid(ContextType).name(), true });
		}

		template<typename ContextType, typename F>
		void SubscribeUnsafe(std::shared_ptr<ContextType> context, F&& func)
		{
			this->listeners.push_back({ std::forward<F>(func), context, typeid(ContextType).name(), true });
		}

		void operator()(Args... args)
		{
			this->CheckRecursion();
			this->isInvoking = true;

			for (size_t i = 0; i < this->listeners.size();)
			{
				auto& cb = this->listeners[i];
				if (cb.hasContext)
				{
					if (cb.context.expired())
					{
						// Swap-and-pop $O(1)$ чистка
						this->listeners[i] = std::move(this->listeners.back());
						this->listeners.pop_back();
						continue; // Не увеличиваем i, чтобы проверить перемещенный элемент
					}
					else
					{
						this->executingSubscriber = cb.subscriberName;
						cb.func(args...);
					}
				}
				else
				{
					this->executingSubscriber = cb.subscriberName;
					cb.func(args...);
				}
				i++;
			}

			this->executingSubscriber = nullptr;
			this->isInvoking = false;
		}
	};

	template<>
	class Event<void> : public eventBase<std::function<void()>>
	{
	public:
		using FuncType = std::function<void()>;
		using EntryType = CallbackEntry<FuncType>;
		using CallbackList = std::vector<EntryType>;

		Event() = default;

		template<typename OwnerType>
		explicit Event(OwnerType* owner) : eventBase<FuncType>(owner) {}

		template<typename F>
		void SubscribeStaticUnsafe(F&& func)
		{
			this->listeners.push_back({ std::forward<F>(func), std::weak_ptr<void>(), "Static / No Context", false });
		}

		template<typename ContextType, typename F>
		void SubscribeUnsafe(std::weak_ptr<ContextType> context, F&& func)
		{
			this->listeners.push_back({ std::forward<F>(func), context, typeid(ContextType).name(), true });
		}

		template<typename ContextType, typename F>
		void SubscribeUnsafe(std::shared_ptr<ContextType> context, F&& func)
		{
			this->listeners.push_back({ std::forward<F>(func), context, typeid(ContextType).name(), true });
		}

		void operator()()
		{
			this->CheckRecursion();
			this->isInvoking = true;

			for (size_t i = 0; i < this->listeners.size();)
			{
				auto& cb = this->listeners[i];
				if (cb.hasContext)
				{
					if (cb.context.expired())
					{
						// Swap-and-pop $O(1)$ чистка
						this->listeners[i] = std::move(this->listeners.back());
						this->listeners.pop_back();
						continue;
					}
					else
					{
						this->executingSubscriber = cb.subscriberName;
						cb.func();
					}
				}
				else
				{
					this->executingSubscriber = cb.subscriberName;
					cb.func();
				}
				i++;
			}

			this->executingSubscriber = nullptr;
			this->isInvoking = false;
		}
	};
}