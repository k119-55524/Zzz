#include "TaskDispatcher.h"
#include <logger/logger.h>

#if defined(_WIN32)
#include <windows.h>
#endif

using namespace zzz::engine;
using namespace zzz::core;
using namespace zzz::templates;

namespace
{
	void SetWorkerPriorityHint(int priorityLevel)
	{
#if defined(_WIN32)
		int winPriority = THREAD_PRIORITY_NORMAL;
		if (priorityLevel > 1)
			winPriority = THREAD_PRIORITY_HIGHEST;
		else if (priorityLevel == 1)
			winPriority = THREAD_PRIORITY_ABOVE_NORMAL;
		else if (priorityLevel == -1)
			winPriority = THREAD_PRIORITY_BELOW_NORMAL;
		else if (priorityLevel < -1)
			winPriority = THREAD_PRIORITY_LOWEST;

		SetThreadPriority(GetCurrentThread(), winPriority);
#else
		(void)priorityLevel;
#endif
	}
}

TaskDispatcher::TaskDispatcher(const TaskDispatcherConfig& config)
{
	// 1. Создание пулов
	if (config.isHeterogeneous)
	{
		if (config.criticalThreads > 0)
		{
			m_CriticalPool = safe_make_unique<ThreadPool>(
				"CriticalWorker",
				config.criticalThreads,
				[](size_t /*id*/) { SetWorkerPriorityHint(2); });
		}

		if (config.primeThreads > 0)
		{
			m_PrimePool = safe_make_unique<ThreadPool>(
				"PrimeWorker",
				config.primeThreads,
				[](size_t /*id*/) { SetWorkerPriorityHint(1); });
		}

		if (config.perfThreads > 0)
		{
			m_PerfPool = safe_make_unique<ThreadPool>(
				"PerfWorker",
				config.perfThreads,
				[](size_t /*id*/) { SetWorkerPriorityHint(0); });
		}

		if (config.effThreads > 0)
		{
			m_EffPool = safe_make_unique<ThreadPool>(
				"EffWorker",
				config.effThreads,
				[](size_t /*id*/) { SetWorkerPriorityHint(-2); });
		}

		// Универсальный выбор пула по списку предпочтений с гарантированным фоллбэком на любой доступный пул
		auto GetFirstAvailablePool = [&](std::initializer_list<ThreadPool*> preference) -> ThreadPool*
		{
			for (auto* pool : preference)
			{
				if (pool)
					return pool;
			}
			for (auto* pool : { m_CriticalPool.get(), m_PrimePool.get(), m_PerfPool.get(), m_EffPool.get() })
			{
				if (pool)
					return pool;
			}
			return nullptr;
		};

		// Заполнение матрицы маршрутизации:
		// Critical: Critical -> Prime -> Perf -> Eff
		m_PoolRouting[static_cast<size_t>(eTaskPriority::Critical)] =
			GetFirstAvailablePool({ m_CriticalPool.get(), m_PrimePool.get(), m_PerfPool.get(), m_EffPool.get() });

		// High: Prime -> Perf -> Critical -> Eff
		m_PoolRouting[static_cast<size_t>(eTaskPriority::High)] =
			GetFirstAvailablePool({ m_PrimePool.get(), m_PerfPool.get(), m_CriticalPool.get(), m_EffPool.get() });

		// Normal: Perf -> Eff -> Prime -> Critical
		m_PoolRouting[static_cast<size_t>(eTaskPriority::Normal)] =
			GetFirstAvailablePool({ m_PerfPool.get(), m_EffPool.get(), m_PrimePool.get(), m_CriticalPool.get() });

		// Background: Eff -> Perf -> Prime -> Critical
		m_PoolRouting[static_cast<size_t>(eTaskPriority::Background)] =
			GetFirstAvailablePool({ m_EffPool.get(), m_PerfPool.get(), m_PrimePool.get(), m_CriticalPool.get() });
	}
	else
	{
		// Однородная топология (SMP) или фоллбэк
		if (config.criticalThreads > 0)
		{
			m_CriticalPool = safe_make_unique<ThreadPool>(
				"CriticalWorker",
				config.criticalThreads,
				[](size_t /*id*/) { SetWorkerPriorityHint(2); });
		}

		uint32_t commonCount = (config.commonThreads > 0) ? config.commonThreads : 1;
		m_CommonPool = safe_make_unique<ThreadPool>(
			"CommonWorker",
			commonCount,
			[](size_t /*id*/) { SetWorkerPriorityHint(0); });

		ThreadPool* criticalTarget = m_CriticalPool ? m_CriticalPool.get() : m_CommonPool.get();
		ThreadPool* generalTarget = m_CommonPool.get();

		m_PoolRouting[static_cast<size_t>(eTaskPriority::Critical)] = criticalTarget;
		m_PoolRouting[static_cast<size_t>(eTaskPriority::High)] = generalTarget;
		m_PoolRouting[static_cast<size_t>(eTaskPriority::Normal)] = generalTarget;
		m_PoolRouting[static_cast<size_t>(eTaskPriority::Background)] = generalTarget;
	}
}

TaskDispatcher::~TaskDispatcher()
{
	JoinAll();
}

eSubmitResult TaskDispatcher::Submit(
	eTaskPriority priority,
	std::function<void()> task,
	std::function<void(std::exception_ptr)> onError)
{
	if (!task)
		return eSubmitResult::RejectedPoolClosed;

	const size_t index = static_cast<size_t>(priority);
	if (index >= m_PoolRouting.size())
		return eSubmitResult::InvalidPriority;

	ThreadPool* pool = m_PoolRouting[index];
	if (!pool)
		return eSubmitResult::RejectedPoolClosed;

	auto safeTask = [task = std::move(task), onError = std::move(onError)]() mutable
	{
		try
		{
			task();
		}
		catch (...)
		{
			if (onError)
			{
				try
				{
					onError(std::current_exception());
				}
				catch (...)
				{
					DOutError("[TaskDispatcher] Исключение внутри пользовательского onError колбэка.");
				}
			}
			else
			{
				DOutError("[TaskDispatcher] Необработанное исключение задачи (onError отсутствует).");
			}
		}
	};

	eEnqueueResult res = pool->Enqueue(std::move(safeTask));
	return (res == eEnqueueResult::Accepted) ? eSubmitResult::Success : eSubmitResult::RejectedPoolClosed;
}

void TaskDispatcher::Join(eTaskPriority priority)
{
	const size_t index = static_cast<size_t>(priority);
	if (index < m_PoolRouting.size() && m_PoolRouting[index])
	{
		m_PoolRouting[index]->Join();
	}
}

void TaskDispatcher::JoinAll()
{
	if (m_CriticalPool)
		m_CriticalPool->Join();
	if (m_PrimePool)
		m_PrimePool->Join();
	if (m_PerfPool)
		m_PerfPool->Join();
	if (m_EffPool)
		m_EffPool->Join();
	if (m_CommonPool)
		m_CommonPool->Join();
}
