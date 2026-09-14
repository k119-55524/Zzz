#pragma once

#include "core/utils/Defines.h"

#if defined(Z_EDITOR)

#include <vector>
#include <thread>
#include "core/hardware/CpuInfo.h"
#include "core/hardware/CpuTopology.h"

namespace zzz::engine
{
	/**
	 * @brief Сохраняет неизменным поведение прежнего PlatformEditor.cpp::GatherHardwareState() (фейковые
	 * данные). Editor намеренно не переведён на реальные Windows-коллекторы в рамках этапа 06 -
	 * см. stage_06_hardware_manager.md, п. 3.5.2.
	 */
	class CpuInfoCollectorEditor final
	{
	public:
		[[nodiscard]] std::vector<::zzz::core::CpuInfo> Collect() const
		{
			using namespace zzz::core;
			return { CpuInfo("Editor CPU", "x64", 1, 1, 1000) };
		}

		[[nodiscard]] ::zzz::core::CpuTopology CollectTopology() const
		{
			using namespace zzz::core;
			CpuTopology topology;
			topology.name = "Editor CPU";
			topology.architecture = "x64";
			const uint32_t cores = std::max(1u, std::thread::hardware_concurrency());
			topology.totalLogicalCores = cores;
			topology.totalPhysicalCores = cores;
			topology.isHeterogeneous = false;
			topology.performanceLogicalCapacity = cores;
			return topology;
		}
	};
}

#endif // defined(Z_EDITOR)
