#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

#include "core/utils/Defines.h"
#include "core/utils/Ensure.h"

namespace zzz::engine
{
	class ResourceManager;

	/**
	 * @class ResourceGarbageCollector
	 * @brief Автономный сервис фоновой очистки неиспользуемых ресурсов (SRP).
	 */
	class ResourceGarbageCollector final
	{
		Z_NO_COPY_MOVE(ResourceGarbageCollector);

	public:
		ResourceGarbageCollector() = delete;
		explicit ResourceGarbageCollector(
			ResourceManager& resourceManager,
			std::chrono::milliseconds interval = std::chrono::milliseconds(10000));
		~ResourceGarbageCollector();

		void Start();
		void Stop();

		/// @brief Приостанавливает фоновую сборку (инкремент счетчика пауз)
		void Pause();

		/// @brief Возобновляет фоновую сборку (декремент счетчика пауз)
		void Resume();

		[[nodiscard]] bool IsPaused() const noexcept { return m_PauseCount.load(std::memory_order_relaxed) > 0; }
		[[nodiscard]] bool IsRunning() const noexcept { return m_IsRunning.load(std::memory_order_relaxed); }

	private:
		void WorkerLoop(std::stop_token stopToken);

		ResourceManager& m_ResourceManager;
		std::chrono::milliseconds m_Interval;

		std::jthread m_Thread;
		std::mutex m_Mutex;
		std::condition_variable_any m_Cv;

		std::atomic<uint32_t> m_PauseCount{ 0 };
		std::atomic<bool> m_IsRunning{ false };
	};

	/**
	 * @class ScopedGCSuspension
	 * @brief RAII-страж приостановки фонового сборщика мусора.
	 */
	class ScopedGCSuspension final
	{
		Z_NO_COPY_MOVE(ScopedGCSuspension);

	public:
		explicit ScopedGCSuspension(ResourceGarbageCollector& gc)
			: m_GC(gc)
		{
			m_GC.Pause();
		}

		~ScopedGCSuspension()
		{
			m_GC.Resume();
		}

	private:
		ResourceGarbageCollector& m_GC;
	};
}
