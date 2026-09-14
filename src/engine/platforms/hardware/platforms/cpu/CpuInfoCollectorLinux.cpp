#include "CpuInfoCollectorLinux.h"

#if defined(Z_LINUX)

#include <fstream>
#include <unistd.h>

using namespace zzz::engine;
using namespace zzz::core;

std::vector<CpuInfo> CpuInfoCollectorLinux::Collect() const
{
	std::string name = "Unknown CPU";
	std::ifstream file("/proc/cpuinfo");
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			if (line.rfind("model name", 0) == 0)
			{
				auto pos = line.find(':');
				if (pos != std::string::npos && pos + 2 <= line.size())
				{
					name = line.substr(pos + 2);
					break;
				}
			}
		}
	}

	long onlineCores = sysconf(_SC_NPROCESSORS_ONLN);
	zU32 coreCount = onlineCores > 0 ? static_cast<zU32>(onlineCores) : 1;

	// Упрощение: физическое и логическое число ядер приравнены (без разбора "physical id"/"core id" из
	// /proc/cpuinfo для учёта Hyper-Threading) - точная реализация отложена до этапа роллаута Linux.
	std::vector<CpuInfo> cpus;
	cpus.emplace_back(std::move(name), "x86_64", coreCount, coreCount, 0);
	return cpus;
}

CpuTopology CpuInfoCollectorLinux::CollectTopology() const
{
	CpuTopology topology;
	topology.architecture = "x64";

	std::string name = "Unknown CPU";
	std::ifstream file("/proc/cpuinfo");
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			if (line.rfind("model name", 0) == 0)
			{
				auto pos = line.find(':');
				if (pos != std::string::npos && pos + 2 <= line.size())
				{
					name = line.substr(pos + 2);
					break;
				}
			}
		}
	}
	topology.name = std::move(name);

	long onlineCores = sysconf(_SC_NPROCESSORS_ONLN);
	zU32 coreCount = onlineCores > 0 ? static_cast<zU32>(onlineCores) : 1;
	topology.totalLogicalCores = coreCount;
	topology.totalPhysicalCores = coreCount;

	// SMP-фоллбэк: если sysfs недоступен, считаем однородным
	topology.performanceLogicalCapacity = coreCount;
	return topology;
}

#endif // defined(Z_LINUX)
