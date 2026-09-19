#pragma once

#include <mutex>
#include <atomic>
#include <vector>
#include <utility>
#include <functional>

#include "core/utils/Ensure.h"

namespace zzz::templates
{
	/**
	 * @brief Потокобезопасная очередь коллбэков для межпоточной передачи задач.
	 *
	 * Фоновые потоки накапливают коллбэки (Push), целевой поток (например, главный цикл Update)
	 * периодически выполняет все накопившиеся задачи (ExecuteAll) со сбросом счётчика.
	 *
	 * Гарантии потокобезопасности:
	 * 1. Накопление (Push) и выполнение (ExecuteAll) полностью потокобезопасны.
	 * 2. Выполнение коллбэков и вызовы их деструкторов происходят СТРОГО вне мьютекса,
	 *    что гарантирует отсутствие взаимных блокировок (дедлоков) и безопасность реентерабельности.
	 * 3. На холостых кадрах проверка наличия задач (m_Count == 0) выполняется мгновенно
	 *    без захвата мьютекса (Zero-Overhead Idle).
	 */
	template <typename CallbackType = std::function<void()>>
	class CallbackQueue final
	{
		Z_NO_COPY_MOVE(CallbackQueue);

	public:
		CallbackQueue() = default;
		~CallbackQueue()
		{
			Clear();
		}

		/// @brief Потокобезопасно добавляет коллбэк в очередь
		void Push(CallbackType callback)
		{
			ensure(callback != nullptr, "Callback не должен быть null");

			{
				std::lock_guard lock(m_Mutex);
				m_Callbacks.push_back(std::move(callback));
				m_Count.fetch_add(1, std::memory_order_release);
			}
		}

		/// @brief Потокобезопасно вызывает все накопленные коллбэки со сбросом счётчика
		void ExecuteAll()
		{
			// Слабая проверка: если задач нет, моментальный выход (hot path кадра)
			if (m_Count.load(std::memory_order_relaxed) == 0) [[likely]]
				return;

			// Сразу захватываем мьютекс
			std::unique_lock lock(m_Mutex);
			if (m_Callbacks.empty()) [[unlikely]]
				return;

			auto ready = std::move(m_Callbacks);
			m_Count.store(0, std::memory_order_relaxed);
			lock.unlock(); // Мгновенно отпускаем мьютекс

			// Вызываем коллбэки строго вне мьютекса (защита от дедлоков и реентерабельности)
			// Изолируем каждый коллбэк, чтобы исключение в одной задаче не прерывало выполнение остальных задач батча
			std::exception_ptr firstException{ nullptr };
			for (auto& cb : ready)
			{
				if (cb)
				{
					try
					{
						cb();
					}
					catch (...)
					{
						if (!firstException)
							firstException = std::current_exception();
					}
				}
			}

			if (firstException)
			{
				std::rethrow_exception(firstException);
			}
		}

		/// @brief Потокобезопасно очищает очередь (деструкторы задач вызываются вне мьютекса)
		void Clear()
		{
			std::vector<CallbackType> toDestroy;
			{
				std::lock_guard lock(m_Mutex);
				toDestroy = std::move(m_Callbacks);
				m_Count.store(0, std::memory_order_relaxed);
			}
		}

	private:
		std::mutex m_Mutex;
		std::vector<CallbackType> m_Callbacks;
		std::atomic<size_t> m_Count{ 0 };
	};
}
