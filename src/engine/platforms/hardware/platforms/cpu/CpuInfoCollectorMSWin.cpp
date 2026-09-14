#include "CpuInfoCollectorMSWin.h"

#if defined(Z_WINDOWS)

using namespace zzz::engine;
using namespace zzz::core;

std::vector<CpuInfo> CpuInfoCollectorMSWin::Collect() const
{
	std::vector<CpuInfo> cpus;

	SYSTEM_INFO sysInfo{};
	GetSystemInfo(&sysInfo);
	cpus.emplace_back("x86 Processor", "x86_64", sysInfo.dwNumberOfProcessors, sysInfo.dwNumberOfProcessors, 0);

	return cpus;
}

CpuTopology CpuInfoCollectorMSWin::CollectTopology() const
{
	CpuTopology topology;

	SYSTEM_INFO sysInfo{};
	GetSystemInfo(&sysInfo);
	topology.name = "x86 Processor";
	topology.architecture = "x64";
	topology.totalLogicalCores = sysInfo.dwNumberOfProcessors;

	DWORD length = 0;
	GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &length);
	if (length > 0)
	{
		std::vector<uint8_t> buffer(length);
		auto* info = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(buffer.data());
		if (GetLogicalProcessorInformationEx(RelationProcessorCore, info, &length))
		{
			uint32_t physicalCores = 0;
			uint32_t pLogical = 0;
			uint32_t eLogical = 0;
			bool hasEfficiencyClass = false;

			uint8_t* ptr = buffer.data();
			while (ptr < buffer.data() + length)
			{
				auto* coreInfo = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(ptr);
				if (coreInfo->Relationship == RelationProcessorCore)
				{
					physicalCores++;
					uint32_t logicalInCore = 0;
					for (WORD g = 0; g < coreInfo->Processor.GroupCount; ++g)
					{
						KAFFINITY mask = coreInfo->Processor.GroupMask[g].Mask;
						while (mask)
						{
							if (mask & 1)
								logicalInCore++;
							mask >>= 1;
						}
					}

					BYTE effClass = coreInfo->Processor.EfficiencyClass;
					if (effClass > 0)
					{
						hasEfficiencyClass = true;
						pLogical += logicalInCore;
					}
					else
					{
						eLogical += logicalInCore;
					}
				}
				ptr += coreInfo->Size;
			}

			topology.totalPhysicalCores = physicalCores > 0 ? physicalCores : sysInfo.dwNumberOfProcessors;
			if (hasEfficiencyClass && eLogical > 0 && pLogical > 0)
			{
				topology.isHeterogeneous = true;
				topology.performanceLogicalCapacity = pLogical;
				topology.efficiencyLogicalCapacity = eLogical;
			}
			else
			{
				topology.isHeterogeneous = false;
				topology.performanceLogicalCapacity = topology.totalLogicalCores;
			}

			return topology;
		}
	}

	topology.totalPhysicalCores = sysInfo.dwNumberOfProcessors;
	topology.isHeterogeneous = false;
	topology.performanceLogicalCapacity = topology.totalLogicalCores;
	return topology;
}

#endif // defined(Z_WINDOWS)
