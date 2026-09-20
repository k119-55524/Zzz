
#include <logger.h>

#include "TaskDispatcher.h"

using namespace zzz::core;
using namespace zzz::engine;
using namespace zzz::templates;

#if defined(Z_MOBILE)
namespace
{
	std::array<uint32_t, 3> CalculateWorkerCounts(const CpuTopology& topology)
	{
		std::array<uint32_t, 3> counts{ 0, 0, 0 };

		// 1. Быстрые ядра в Normal: забираем 1 или 2 потока ТОЛЬКО если есть свободные P-ядра после Critical
		if (topology.performanceLogicalCapacity > 2)
		{
			counts[1] = std::min(topology.performanceLogicalCapacity - 2, 2u);
		}

		// 2. Энергоэффективные ядра в Background: забираем максимум E-ядер минус 1 (резерв ОС)
		if (topology.efficiencyLogicalCapacity > 1)
		{
			counts[2] = topology.efficiencyLogicalCapacity - 1;
		}

		// 3. Страховка: забираем 1 или 2 потока (total - 2 Critical - 1 резерв, но не более 2)
		if (counts[0] == 0 && counts[1] == 0 && counts[2] == 0)
		{
			counts[1] = std::min(topology.totalLogicalCores - 2 - 1, 2u);
		}

		return counts;
	}
}
#else
namespace
{
	std::array<uint32_t, 3> CalculateWorkerCounts(const CpuTopology& topology)
	{
		std::array<uint32_t, 3> counts = {
			(topology.primeLogicalCapacity > 3) ? (topology.primeLogicalCapacity - 2 - 1) : 0u,
			(topology.performanceLogicalCapacity > 1) ? (topology.performanceLogicalCapacity - 1) : 0u,
			(topology.efficiencyLogicalCapacity > 1) ? (topology.efficiencyLogicalCapacity - 1) : 0u
		};

		// Страховка (однородный CPU): забираем все ядра за вычетом 2 Critical и резерва ОС (1 или 2)
		if (counts[0] == 0 && counts[1] == 0 && counts[2] == 0)
		{
			const uint32_t systemReserve = (topology.totalLogicalCores >= 6) ? 2 : 1;
			counts[1] = topology.totalLogicalCores - 2 - systemReserve;
		}

		return counts;
	}
}
#endif

TaskDispatcher::TaskDispatcher(const CpuTopology& topology)
	: m_Pools{}
{
	InitializePools(topology);
}

void TaskDispatcher::InitializePools(const CpuTopology& topology)
{
	const uint32_t logicalCores = topology.totalLogicalCores;
	if (logicalCores < 4)
		THROW_RUNTIME("Неподдерживаемая аппаратная конфигурация CPU. Требуется минимум 4 логических ядра (обнаружено: {}).", logicalCores);

	// 1. Нулевой пул Critical: гарантированно создаётся на 2 кадровых потока
	m_Pools[static_cast<size_t>(eTaskPriority::Critical)] = safe_make_shared<ThreadPool>(
		"CriticalWorker",
		2,
		eThreadPriority::Critical);

	// 2. Массив квот пулов [0] High, [1] Normal, [2] Background
	const std::array<uint32_t, 3> counts = CalculateWorkerCounts(topology);

	constexpr std::array<const char*, 3> poolNames = {"HighWorker", "NormalWorker", "BackgroundWorker"};

	// 3. Цикл создания рабочих пулов: создаём только там, где выделены потоки
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
	DOut(Hardware, "  Quota -> Critical: 2, High: {}, Normal: {}, Background: {}",
		counts[0], counts[1], counts[2]);
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

bool TaskDispatcher::Submit(eTaskPriority priority, std::function<void()> task, ErrorHandler onError)
{
	ensure(task != nullptr, "Попытка отправить пустую задачу в TaskDispatcher.");

	const size_t index = static_cast<size_t>(priority);
	ensure(index < m_Pools.size(), "Некорректный приоритет задачи.");

	const auto& pool = m_Pools[index];
	ensure(pool != nullptr, "Физический пул для задачи не инициализирован.");

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

	return pool->Enqueue(std::move(safeTask)) == eEnqueueResult::Accepted;
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
