#pragma once

#include "core/hardware/CpuTopology.h"
#include "engine/tasks/TaskDispatcherConfig.h"
#include "engine/launch/EngineLaunchOptions.h"
#include <algorithm>

namespace zzz::engine
{
	/**
	 * @brief Платформенная политика распределения потоков для Mobile (Android, iOS).
	 * Включает жесткий лимит перегрева (Mobile Hard Cap <= 4 воркера).
	 */
	class PlatformTaskPolicyMobile final
	{
	public:
		[[nodiscard]] static TaskDispatcherConfig Resolve(
			const ::zzz::core::CpuTopology& topology,
			const EngineLaunchOptions& launchOptions)
		{
			TaskDispatcherConfig config;
			const uint32_t logicalCores = topology.totalLogicalCores;

			// Базовый бюджет воркеров
			uint32_t totalWorkersBudget = (logicalCores >= 6) ? (logicalCores - 2) : std::max(1u, logicalCores - 1);

			// Пользовательский CLI лимит
			if (launchOptions.threadCap > 0)
			{
				totalWorkersBudget = std::min(totalWorkersBudget, launchOptions.threadCap);
			}

			// Mobile Hard Cap (не более 4 активных воркеров во избежание троттлинга)
			totalWorkersBudget = std::min(totalWorkersBudget, 4u);

			if (totalWorkersBudget <= 1)
			{
				config.isHeterogeneous = false;
				config.criticalThreads = 0;
				config.commonThreads = 1;
				return config;
			}

			if (topology.isHeterogeneous && topology.performanceLogicalCapacity > 0)
			{
				config.isHeterogeneous = true;
				config.criticalThreads = 1;
				uint32_t remainingBudget = totalWorkersBudget - config.criticalThreads;

				if (topology.primeLogicalCapacity > 0 && remainingBudget > 0)
				{
					config.primeThreads = 1;
					remainingBudget -= 1;
				}

				if (remainingBudget > 0)
				{
					config.perfThreads = std::min(remainingBudget, (remainingBudget > 1) ? 2u : 1u);
					remainingBudget -= config.perfThreads;
				}

				if (remainingBudget > 0)
				{
					config.effThreads = remainingBudget;
				}

				return config;
			}

			config.isHeterogeneous = false;
			config.criticalThreads = 1;
			config.commonThreads = totalWorkersBudget - 1;
			return config;
		}
	};
}
