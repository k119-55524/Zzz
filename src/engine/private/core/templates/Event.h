#pragma once

#include <vector>
#include <memory>
#include <typeinfo>
#include <functional>
#include <foundation.h>
#include "../../../headers/throwWrappers.h"

namespace zzz::engine
{
	template<typename FuncType>
	struct CallbackEntry
	{
		FuncType func;
		std::weak_ptr<void> context;
		const char* subscriberName = "Unknown";
		bool hasContext = false;
	};

	template<typename FuncType>
	class EventBase
	{
	protected:
		using EntryType = CallbackEntry<FuncType>;
		using CallbackList = std::vector<EntryType>;

		CallbackList listeners;

		const char* ownerName = "Unknown";
		const char* executingSubscriber = nullptr;

		bool isInvoking = false;
		bool* destroyFlag = nullptr;

	public:
		EventBase() = default;

		template<typename OwnerType>
		explicit EventBase(OwnerType* owner)
		{
			if (owner)
				ownerName = typeid(*owner).name();
		}

		EventBase(const EventBase&) = delete;
		EventBase& operator=(const EventBase&) = delete;
		EventBase(EventBase&&) = delete;
		EventBase& operator=(EventBase&&) = delete;

	protected:
		~EventBase()
		{
			if (destroyFlag)
				*destroyFlag = true;
		}

		void CheckRecursion()
		{
			if (isInvoking)
			{
				THROW_RUNTIME(
					"[FATAL ERROR] Recursive invocation detected!\n"
					"Event Owner: {}\n"
					"Executing Subscriber: {}",
					ownerName,
					executingSubscriber ? executingSubscriber : "Unknown");
			}
		}

		void CheckModifying()
		{
			if (isInvoking)
			{
				THROW_RUNTIME(
					"[FATAL ERROR] Cannot modify Event while invoking!\n"
					"Event Owner: {}\n"
					"Executing Subscriber: {}",
					ownerName,
					executingSubscriber ? executingSubscriber : "Unknown");
			}
		}

		struct InvocationGuard
		{
			EventBase* event;

			explicit InvocationGuard(EventBase* e, bool* destroyed)
				: event(e)
			{
				event->CheckRecursion();

				event->destroyFlag = destroyed;
				event->isInvoking = true;
			}

			~InvocationGuard()
			{
				if (event->destroyFlag && !(*event->destroyFlag))
				{
					event->executingSubscriber = nullptr;
					event->isInvoking = false;
					event->destroyFlag = nullptr;
				}
			}

			InvocationGuard(const InvocationGuard&) = delete;
			InvocationGuard& operator=(const InvocationGuard&) = delete;
		};
	};

	template<typename... Args>
	class Event final : public EventBase<std::function<void(Args...)>>
	{
	private:
		using Base = EventBase<std::function<void(Args...)>>;

	public:
		using FuncType = std::function<void(Args...)>;
		using EntryType = CallbackEntry<FuncType>;

		Event() = default;

		template<typename OwnerType>
		explicit Event(OwnerType* owner)
			: Base(owner)
		{}

		void Clear()
		{
			this->CheckModifying();
			this->listeners.clear();
		}

		void Reserve(size_t count)
		{
			this->CheckModifying();
			this->listeners.reserve(count);
		}

		template<typename F>
		void SubscribeStatic(F&& func)
		{
			this->CheckModifying();

			FuncType f(std::forward<F>(func));
			if (!f)
			{
				THROW_RUNTIME("[FATAL ERROR] Cannot subscribe an empty callback to Event!\nEvent Owner: {}", this->ownerName);
			}

			this->listeners.push_back({
				std::move(f),
				{},
				"Static",
				false
				});
		}

		template<typename ContextType, typename F>
		void Subscribe(std::weak_ptr<ContextType> context, F&& func)
		{
			this->CheckModifying();

			FuncType f(std::forward<F>(func));
			if (!f)
			{
				THROW_RUNTIME("[FATAL ERROR] Cannot subscribe an empty callback to Event!\nEvent Owner: {}", this->ownerName);
			}

			this->listeners.push_back({
				std::move(f),
				context,
				typeid(ContextType).name(),
				true
				});
		}

		template<typename ContextType, typename F>
		void Subscribe(std::shared_ptr<ContextType> context, F&& func)
		{
			Subscribe(std::weak_ptr<ContextType>(context),
				std::forward<F>(func));
		}

		/// @brief Вызывает всех подписчиков события.
		/// @note Порядок вызова подписчиков не гарантируется (из-за оптимизированного O(1) удаления протухших подписок с помощью swap and pop).
		/// @param args Аргументы, которые будут переданы всем слушателям.
		void operator()(Args... args)
		{
			bool destroyed = false;

			typename Base::InvocationGuard guard(this, &destroyed);

			for (size_t i = 0; i < this->listeners.size();)
			{
				auto& entry = this->listeners[i];

				if (entry.hasContext && entry.context.expired())
				{
					if (i != this->listeners.size() - 1)
					{
						entry = std::move(this->listeners.back());
					}
					this->listeners.pop_back();
					continue;
				}

				auto func = entry.func;
				auto subscriber = entry.subscriberName;

				this->executingSubscriber = subscriber;

				func(args...);

				if (destroyed)
					return;

				++i;
			}
		}
	};
}