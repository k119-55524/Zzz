
#include <logger.h>

#include "TaskDispatcher.h"

using namespace zzz::core;
using namespace zzz::engine;
using namespace zzz::templates;

TaskDispatcher::TaskDispatcher(const CpuTopology& topology)
{
	InitializePools(topology);
}

void TaskDispatcher::InitializePools(const CpuTopology& topology)
{
	const uint32_t logicalCores = topology.totalLogicalCores;
	if (logicalCores < 4)
		THROW_RUNTIME("Неподдерживаемая аппаратная конфигурация CPU. Требуется минимум 4 логических ядра (обнаружено: {}).", logicalCores);

	// 1. Нулевой пул Critical: гарантированно создаётся сразу на 2 кадровых потока
	m_Pools[static_cast<size_t>(eTaskPriority::Critical)] = safe_make_shared<ThreadPool>(
		"CriticalWorker",
		c_DefaultCriticalThreads,
		eThreadPriority::Critical);

	// 2. Массив доступных ядер по кластерам: [0] High (Prime), [1] Normal (Perf), [2] Background (Eff)
	// В первой строчке сразу -2 (Critical) и -1 (резерв), в остальных по -1 (резерв).
	std::array<uint32_t, 3> counts = {
		(topology.primeLogicalCapacity > 3) ? (topology.primeLogicalCapacity - 2 - 1) : 0u,
		(topology.performanceLogicalCapacity > 1) ? (topology.performanceLogicalCapacity - 1) : 0u,
		(topology.efficiencyLogicalCapacity > 1) ? (topology.efficiencyLogicalCapacity - 1) : 0u
	};

	constexpr std::array<const char*, 3> poolNames = {"HighWorker", "NormalWorker", "BackgroundWorker"};

	// 3. Цикл создания пулов: создаём только там, где остались ядра
	for (size_t i = 0; i < counts.size(); ++i)
	{
		if (counts[i] > 0)
			m_Pools[i + 1] = safe_make_shared<ThreadPool>(poolNames[i], counts[i], static_cast<eThreadPriority>(i + 1));
	}

	// 4. Копирование влево среди рабочих пулов [1..3] (чтобы не забирать Critical [0], если справа есть ядра)
	for (size_t i = m_Pools.size() - 1; i > 1; --i)
	{
		if (!m_Pools[i - 1] && m_Pools[i])
			m_Pools[i - 1] = m_Pools[i];
	}

	// 5. Копирование вправо: если пул всё ещё пуст, подтягиваем ближайший левый
	for (size_t i = 1; i < m_Pools.size(); ++i)
	{
		if (!m_Pools[i])
			m_Pools[i] = m_Pools[i - 1];
	}

#if Z_ADD_LOGGER
	DOut(Hardware, "========== [TaskDispatcher] Workers Allocation ==========");
	DOut(Hardware, "  Logical cores: {}", logicalCores);
	DOut(Hardware, "  Quota -> High: {}, Normal: {}, Background: {}", counts[0], counts[1], counts[2]);
	for (size_t i = 0; i < m_Pools.size(); ++i)
	{
		const auto prio = static_cast<eTaskPriority>(i);
		size_t aliasOf = i;
		for (size_t j = 0; j < i; ++j)
		{
			if (m_Pools[j] == m_Pools[i])
			{
				aliasOf = j;
				break;
			}
		}

		if (aliasOf != i)
		{
			DOut(Hardware, "  [{}] -> Shared with [{}]", ToString(prio), ToString(static_cast<eTaskPriority>(aliasOf)));
		}
		else
		{
			DOut(Hardware, "  [{}] -> Dedicated pool ({} threads)",
				ToString(prio), m_Pools[i]->GetThreadCount());
		}
	}
	DOut(Hardware, "==========================================================");
#endif
}

TaskDispatcher::~TaskDispatcher()
{
	JoinAll();
}

eSubmitResult TaskDispatcher::Submit(eTaskPriority priority, std::function<void()> task, ErrorHandler onError)
{
	if (!task)
		return eSubmitResult::RejectedPoolClosed;

	const size_t index = static_cast<size_t>(priority);
	ensure(index < m_Pools.size(), "Некорректный приоритет задачи.");

	const auto& pool = m_Pools[index];
	if (!pool)
		return eSubmitResult::RejectedPoolClosed;

	auto safeTask = [userTask = std::move(task), userOnError = std::move(onError)]() noexcept
	{
		try
		{
			userTask();
		}
		catch (...)
		{
			const std::exception_ptr ex = std::current_exception();
			DOutError("Исключение при выполнении задачи в TaskDispatcher.");

			if (userOnError)
			{
				try
				{
					userOnError(ex);
				}
				catch (...)
				{
					DOutError("Исключение внутри обработчика onError в TaskDispatcher.");
				}
			}
		}
	};

	const eEnqueueResult res = pool->Enqueue(std::move(safeTask));
	return (res == eEnqueueResult::Accepted) ? eSubmitResult::Success : eSubmitResult::RejectedPoolClosed;
}

void TaskDispatcher::Join(eTaskPriority priority)
{
	const size_t index = static_cast<size_t>(priority);
	ensure(index < m_Pools.size(), "Некорректный приоритет задачи.");

	if (m_Pools[index])
	{
		m_Pools[index]->Join();
	}
}

void TaskDispatcher::JoinAll()
{
	for (size_t i = 0; i < m_Pools.size(); ++i)
	{
		if (!m_Pools[i])
			continue;

		bool alreadyJoined = false;
		for (size_t j = 0; j < i; ++j)
		{
			if (m_Pools[j] == m_Pools[i])
			{
				alreadyJoined = true;
				break;
			}
		}

		if (!alreadyJoined)
			m_Pools[i]->Join();
	}
}
