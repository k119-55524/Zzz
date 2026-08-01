#pragma once

#include <mutex>
#include <vector>
#include <memory>
#include <atomic>
#include <functional>
#include <common/Common.h>
#include <common/ThrowWrappers.h>
#include <logger/logger.h>

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
		size_t invokingCount = 0;
		std::atomic<bool> needsCleanup = false;
		
		struct InvocationGuard;
		InvocationGuard* activeGuards = nullptr; // Интрузивный список для безопасного удаления

	public:
		EventBase() = default;

		EventBase(const EventBase&) = delete;
		EventBase& operator=(const EventBase&) = delete;
		EventBase(EventBase&&) = delete;
		EventBase& operator=(EventBase&&) = delete;

	protected:
		~EventBase()
		{
			// Блокируем мьютекс, чтобы безопасно пройтись по списку гардов.
			std::lock_guard<std::mutex> lock(eventMutex);
			
			// Если объект события удаляется прямо во время вызова колбэка, мы проходим по всем 
			// активным вызовам (их может быть несколько при рекурсии или многопоточности)
			// и поднимаем у них флаг. Это заставит их немедленно прервать цикл и безопасно выйти.
			InvocationGuard* curr = activeGuards;
			while (curr)
			{
				*(curr->destroyed) = true;
				curr = curr->next;
			}
		}

		void CheckModifying()
		{
			if (invokingCount > 0)
				THROW_RUNTIME("[КРИТИЧЕСКАЯ ОШИБКА] Невозможно изменить Event во время вызова!");
		}

		/// @brief RAII-обертка для безопасного вызова события.
		/// Выполняет три функции:
		/// 1. Гарантирует откат счетчика invokingCount даже при выбросе исключений (Exception Safety).
		/// 2. Служит узлом интрузивного списка для безопасного удаления события (destroyFlag).
		/// 3. Применяет отложенные изменения (добавление/очистку подписчиков), когда счетчик вызовов падает до 0.
		struct InvocationGuard
		{
			EventBase* event;
			bool* destroyed;
			InvocationGuard* next;

			/// @brief Захватывает мьютекс, увеличивает счетчик активных вызовов 
			/// и регистрирует текущий вызов в цепочке активных гардов (activeGuards).
			explicit InvocationGuard(EventBase* e, bool* d)
				: event(e), destroyed(d), next(nullptr)
			{
				std::lock_guard<std::mutex> lock(event->eventMutex);
				
				// Добавляем текущий вызов в голову интрузивного списка.
				// Список нужен для того, чтобы деструктор ~EventBase мог безопасно 
				// прервать работу всех потоков при удалении объекта из памяти.
				this->next = event->activeGuards;
				event->activeGuards = this;
				
				event->invokingCount++;
			}

			/// @brief При штатном завершении вызова (или при раскрутке стека из-за исключения):
			/// 1. Убирает себя из списка активных гардов.
			/// 2. Уменьшает счетчик вызовов.
			/// 3. Если счетчик упал до 0 (последний вызов завершен), производит 
			///    реальное удаление "мертвых" подписчиков и вливает отложенные подписки.
			~InvocationGuard()
			{
				if (*destroyed)
					return;

				std::lock_guard<std::mutex> lock(event->eventMutex);

				// Вызов завершился штатно — удаляем себя из списка активных гардов
				InvocationGuard** curr = &event->activeGuards;
				while (*curr)
				{
					if (*curr == this)
					{
						*curr = this->next;
						break;
					}
					curr = &(*curr)->next;
				}

				event->invokingCount--;
				if (event->invokingCount == 0)
				{
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

					if (this->invokingCount == 0)
					{
						entry.func = nullptr;
						entry.context.reset();
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
				THROW_RUNTIME("[КРИТИЧЕСКАЯ ОШИБКА] Невозможно подписать пустой колбэк на Event!");

			std::lock_guard<std::mutex> lock(this->eventMutex);
			if (this->invokingCount > 0)
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

			size_t count = this->listeners.size();
			for (size_t i = 0; i < count; ++i)
			{
				auto& entry = this->listeners[i];
				if (entry.isDead.load() || entry.context.expired())
					continue;

				try
				{
					entry.func(args...);
				}
				catch (const std::exception& e)
				{
					DOutException("[Event] Исключение в колбэке: {}. Подписчик будет отписан.", e.what());
					entry.isDead.store(true);
					this->needsCleanup.store(true);
				}
				catch (...)
				{
					DOutException("[Event] Неизвестное исключение в колбэке. Подписчик будет отписан.");
					entry.isDead.store(true);
					this->needsCleanup.store(true);
				}

				if (destroyed)
					return;
			}
		}

	private:
		using Base = EventBase<std::function<void(Args...)>, Ordered>;
	};

	/// @brief Упорядоченное событие. Гарантирует сохранение порядка подписчиков 
	/// (кто подписался первым, тот вызывается первым). 
	/// При очистке использует сдвиг элементов (чуть медленнее).
	template<typename... Args>
	using Event = EventImpl<true, Args...>;

	/// @brief Неупорядоченное событие. Порядок вызова подписчиков не гарантируется. 
	/// При очистке меняет удаляемый элемент местами с последним (работает за O(1), быстрее).
	template<typename... Args>
	using UnorderedEvent = EventImpl<false, Args...>;
}
