#pragma once

#include "core/CoreIncludes.h"
#include "core/templates/ThreadPool.h"
#include "engine/tasks/TaskPriority.h"
#include "engine/tasks/TaskDispatcherConfig.h"
#include <array>
#include <memory>
#include <functional>
#include <exception>

namespace zzz::engine
{
	enum class eSubmitResult : uint8_t
	{
		Success,
		RejectedPoolClosed,
		InvalidPriority
	};

	/**
	 * @brief Централизованный диспетчер пулов потоков движка.
	 * Обеспечивает O(1) маршрутизацию задач по приоритетам и clean shutdown через JoinAll().
	 */
	class TaskDispatcher final
	{
		Z_NO_COPY_MOVE(TaskDispatcher);

	public:
		explicit TaskDispatcher(const TaskDispatcherConfig& config);
		~TaskDispatcher();

		eSubmitResult Submit(
			eTaskPriority priority,
			std::function<void()> task,
			std::function<void(std::exception_ptr)> onError = nullptr);

		void Join(eTaskPriority priority);
		void JoinAll();

	private:
		static_assert(static_cast<size_t>(eTaskPriority::Count) == 4, "m_PoolRouting size mismatch!");
		std::array<zzz::templates::ThreadPool*, static_cast<size_t>(eTaskPriority::Count)> m_PoolRouting{};

		std::unique_ptr<zzz::templates::ThreadPool> m_CriticalPool;
		std::unique_ptr<zzz::templates::ThreadPool> m_PrimePool;
		std::unique_ptr<zzz::templates::ThreadPool> m_PerfPool;
		std::unique_ptr<zzz::templates::ThreadPool> m_EffPool;
		std::unique_ptr<zzz::templates::ThreadPool> m_CommonPool;
	};
}
