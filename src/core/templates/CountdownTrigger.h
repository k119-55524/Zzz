#pragma once

#include <atomic>
#include <functional>
#include <utility>

#include "core/utils/Ensure.h"
#include "core/utils/macros/MiscMacros.h"

namespace zzz::templates
{
	/**
	 * @class CountdownTrigger
	 * @brief Неблокирующий триггер завершения группы параллельных/асинхронных операций (Non-blocking Countdown Barrier).
	 *
	 * @details Предназначен для реактивной синхронизации набора асинхронных задач без блокировки рабочих потоков.
	 * В отличие от стандартного `std::latch` из C++20, который блокирует вызывающий поток через метод `wait()`,
	 * `CountdownTrigger` оперирует исключительно неблокирующим декрементом (`CountDown()`) и однократно вызывает
	 * целевой коллбэк (`onComplete`), когда счётчик оставшихся операций достигает нуля.
	 *
	 * Архитектурные свойства и гарантии:
	 * 1. **Реактивность и отсутствие сна (Zero-Sleep / Non-blocking):**
	 *    Потоки пула задач (`ThreadPool`) не усыпляются в ожидании соседей, а сразу освобождаются для исполнения
	 *    следующих задач очереди. Тот поток, чей вызов `CountDown()` уменьшил счётчик до нуля, выполняет коллбэк.
	 * 2. **Потокобезопасность и строгий Memory Ordering:**
	 *    Счётчик управляется атомарной операцией `fetch_sub` с семантикой `std::memory_order_acq_rel`. Это гарантирует,
	 *    что все модификации памяти (данные слоёв, мешей, ресурсов), выполненные рабочими потоками до вызова `CountDown()`,
	 *    становятся строго видимы внутри финального коллбэка `onComplete`.
	 * 3. **Однократность срабатывания (Run-Once Guarantee):**
	 *    Коллбэк гарантированно вызывается ровно один раз, что исключает повторную передачу сцены/ресурса в конвейер.
	 * 4. **Корректная обработка граничного случая (Zero Count):**
	 *    Если количество операций изначально равно 0 (например, пустая сцена без слоёв или материал без внешних текстур),
	 *    коллбэк немедленно исполняется прямо в конструкторе, предотвращая вечный hang асинхронного конвейера.
	 * 5. **Контрактная защита (Fail-Fast):**
	 *    Попытка вызвать `CountDown()` сверх установленного лимита (декремент ниже 0) или передать пустой коллбэк
	 *    немедленно прерывается через `ensure`.
	 *
	 * Типичные сценарии использования в ZzzEngine:
	 * - Асинхронное создание и наполнение слоёв сцены (`SceneManager` / `Scene::Initialize`);
	 * - Параллельная загрузка сабмешей многосоставной геометрии (`MultiMesh` в этапе 20);
	 * - Ожидание завершения GPU-загрузки набора текстур материала (Albedo, Normal, Roughness в этапе 21);
	 * - Стриминг и распаковка составных чанков пакетов ресурсов (`package.dat` / `data.dat`).
	 */
	class CountdownTrigger final
	{
		Z_NO_COPY_MOVE(CountdownTrigger);

	public:
		/**
		 * @brief Создаёт триггер с начальным числом ожидаемых операций и целевым коллбэком.
		 * @param initialCount Количество операций, завершения которых необходимо дождаться.
		 * @param onComplete Коллбэк, вызываемый в момент завершения последней операции.
		 */
		CountdownTrigger(size_t initialCount, std::function<void()> onComplete)
			: m_Remaining(initialCount)
			, m_OnComplete(std::move(onComplete))
		{
			ensure(m_OnComplete != nullptr, "CountdownTrigger: коллбэк onComplete не должен быть null");

			// Граничный случай: задач изначально нет — немедленно вызываем коллбэк
			if (initialCount == 0)
			{
				m_OnComplete();
			}
		}

		~CountdownTrigger() = default;

		/**
		 * @brief Потокобезопасно уменьшает счётчик оставшихся операций на 1.
		 *
		 * @details Если текущий вызов свёл счётчик к 0, вызывается целевой коллбэк.
		 * Вызов защищён контрактом от декремента сверх установленного начального значения.
		 */
		void CountDown()
		{
			const size_t prev = m_Remaining.fetch_sub(1, std::memory_order_acq_rel);
			ensure(prev > 0, "CountdownTrigger::CountDown: вызов декремента сверх установленного начального лимита");

			if (prev == 1)
			{
				m_OnComplete();
			}
		}

		/**
		 * @brief Возвращает текущее количество оставшихся операций.
		 * @note Значение носит информационный характер из-за возможных параллельных вызовов.
		 */
		[[nodiscard]] size_t GetRemaining() const noexcept
		{
			return m_Remaining.load(std::memory_order_relaxed);
		}

		/**
		 * @brief Проверяет, завершились ли все операции.
		 */
		[[nodiscard]] bool IsCompleted() const noexcept
		{
			return GetRemaining() == 0;
		}

	private:
		std::atomic<size_t>   m_Remaining;
		std::function<void()> m_OnComplete;
	};
}
