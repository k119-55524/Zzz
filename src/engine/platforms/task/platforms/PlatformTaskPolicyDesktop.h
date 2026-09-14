#pragma once

#include "core/hardware/CpuTopology.h"
#include "engine/tasks/TaskDispatcherConfig.h"
#include "engine/launch/EngineLaunchOptions.h"
#include <algorithm>

namespace zzz::engine
{
	/**
	 * @brief Платформенная политика распределения потоков для Desktop (Windows, Linux, macOS).
	 */
	class PlatformTaskPolicyDesktop final
	{
	public:
		[[nodiscard]] static TaskDispatcherConfig Resolve(
			const ::zzz::core::CpuTopology& topology,
			const EngineLaunchOptions& launchOptions)
		{
			TaskDispatcherConfig config;
			const uint32_t logicalCores = topology.totalLogicalCores;

			// Расчет общего базового бюджета воркеров
			uint32_t totalWorkersBudget = (logicalCores >= 6) ? (logicalCores - 2) : std::max(1u, logicalCores - 1);

			// Учет пользовательского лимита CLI (--threads=N)
			if (launchOptions.threadCap > 0)
			{
				totalWorkersBudget = std::min(totalWorkersBudget, launchOptions.threadCap);
			}

			// Граничный случай: одноядерный фоллбэк / TotalWorkersBudget <= 1
			if (totalWorkersBudget <= 1)
			{
				config.isHeterogeneous = false;
				config.criticalThreads = 0;
				config.commonThreads = 1;
				return config;
			}

			// Проверяем наличие гетерогенной топологии (P/E-ядра или Prime-ядра)
			if (topology.isHeterogeneous && topology.performanceLogicalCapacity > 0)
			{
				config.isHeterogeneous = true;

				// CriticalPool выделяется всегда (минимум 1, стандартно 2 воркера)
				config.criticalThreads = (totalWorkersBudget >= 4) ? 2 : 1;
				uint32_t remainingBudget = totalWorkersBudget - config.criticalThreads;

				// Кластер супер-ядер (Prime), если присутствует
				if (topology.primeLogicalCapacity > 0 && remainingBudget > 0)
				{
					config.primeThreads = std::min(topology.primeLogicalCapacity, remainingBudget);
					remainingBudget -= config.primeThreads;
				}

				// Кластер Performance-ядер (P-ядра)
				uint32_t perfTarget = (topology.performanceLogicalCapacity > config.criticalThreads) ?
					(topology.performanceLogicalCapacity - config.criticalThreads) : topology.performanceLogicalCapacity;
				config.perfThreads = std::min(perfTarget, remainingBudget);
				remainingBudget -= config.perfThreads;

				// Кластер Efficiency-ядер (E-ядра)
				if (topology.efficiencyLogicalCapacity > 0 && remainingBudget > 0)
				{
					config.effThreads = std::min(topology.efficiencyLogicalCapacity, remainingBudget);
					remainingBudget -= config.effThreads;
				}

				// Если после раздачи по ёмкостям остался свободный бюджет, отдаем в perfThreads
				if (remainingBudget > 0)
				{
					config.perfThreads += remainingBudget;
				}

				return config;
			}

			// Однородная (SMP) архитектура
			config.isHeterogeneous = false;
			config.criticalThreads = (totalWorkersBudget >= 4) ? 2 : 1;
			config.commonThreads = totalWorkersBudget - config.criticalThreads;
			if (config.commonThreads == 0)
				config.commonThreads = 1;

			return config;
		}
	};
}
