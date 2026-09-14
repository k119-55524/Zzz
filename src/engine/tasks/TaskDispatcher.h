#pragma once

#include <array>
#include <memory>
#include <functional>

#include "core/hardware/CpuTopology.h"
#include "core/templates/ThreadPool.h"
#include "engine/tasks/TaskPriority.h"

namespace zzz::engine
{
	using namespace zzz::core;
	using namespace zzz::templates;

	enum class eSubmitResult : uint8_t
	{
		Success,
		RejectedPoolClosed
	};

	/**
	 * @brief Централизованный диспетчер пулов потоков движка.
	 * Обеспечивает O(1) маршрутизацию задач по приоритетам и clean shutdown через JoinAll().
	 */
	class TaskDispatcher final
	{
		Z_NO_COPY_MOVE(TaskDispatcher);

	public:
		using ErrorHandler = std::function<void(std::exception_ptr)>;

		explicit TaskDispatcher(const CpuTopology& topology);
		~TaskDispatcher();

		eSubmitResult Submit(eTaskPriority priority, std::function<void()> task, ErrorHandler onError = nullptr);
		void Join(eTaskPriority priority);
		void JoinAll();

	private:
		void InitializePools(const CpuTopology& topology);

		static constexpr uint32_t c_DefaultCriticalThreads = 2;

		static_assert(static_cast<size_t>(eTaskPriority::Count) == 4, "m_Pools size mismatch!");
		static_assert(static_cast<size_t>(eTaskPriority::Count) == static_cast<size_t>(eThreadPriority::Count),
			"eTaskPriority and eThreadPriority size mismatch!");
		std::array<std::shared_ptr<ThreadPool>, static_cast<size_t>(eTaskPriority::Count)> m_Pools{};
	};
}
