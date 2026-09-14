#include "CpuInfoCollectorApple.h"

#if defined(Z_APPLE)

#include <sys/sysctl.h>

using namespace zzz::engine;
using namespace zzz::core;

namespace
{
	std::string SysctlString(const char* name)
	{
		std::size_t size = 0;
		if (sysctlbyname(name, nullptr, &size, nullptr, 0) != 0 || size == 0)
			return {};

		std::string result(size, '\0');
		if (sysctlbyname(name, result.data(), &size, nullptr, 0) != 0)
			return {};

		while (!result.empty() && result.back() == '\0')
			result.pop_back();

		return result;
	}

	zU32 SysctlU32(const char* name, zU32 fallback)
	{
		int value = 0;
		std::size_t size = sizeof(value);
		if (sysctlbyname(name, &value, &size, nullptr, 0) != 0 || value <= 0)
			return fallback;

		return static_cast<zU32>(value);
	}
}

std::vector<CpuInfo> CpuInfoCollectorApple::Collect() const
{
	std::string name = SysctlString("machdep.cpu.brand_string");
	if (name.empty())
		name = "Apple CPU";

	zU32 physicalCores = SysctlU32("hw.physicalcpu", 1);
	zU32 logicalCores = SysctlU32("hw.logicalcpu", physicalCores);

	std::vector<CpuInfo> cpus;
	cpus.emplace_back(std::move(name), "arm64", physicalCores, logicalCores, 0);
	return cpus;
}

CpuTopology CpuInfoCollectorApple::CollectTopology() const
{
	CpuTopology topology;
	std::string name = SysctlString("machdep.cpu.brand_string");
	if (name.empty())
		name = "Apple CPU";

	topology.name = std::move(name);
	topology.architecture = "arm64";
	topology.totalPhysicalCores = SysctlU32("hw.physicalcpu", 1);
	topology.totalLogicalCores = SysctlU32("hw.logicalcpu", topology.totalPhysicalCores);

	zU32 nperflevels = SysctlU32("hw.nperflevels", 1);
	if (nperflevels > 1)
	{
		// Apple Silicon: perflevel0 = Performance cores, perflevel1 = Efficiency cores
		zU32 pCores = SysctlU32("hw.perflevel0.logicalcpu", 0);
		zU32 eCores = SysctlU32("hw.perflevel1.logicalcpu", 0);

		if (pCores > 0 && eCores > 0)
		{
			topology.performanceLogicalCapacity = pCores;
			topology.efficiencyLogicalCapacity = eCores;
			return topology;
		}
	}

	topology.performanceLogicalCapacity = topology.totalLogicalCores;
	return topology;
}

#endif // defined(Z_APPLE)
