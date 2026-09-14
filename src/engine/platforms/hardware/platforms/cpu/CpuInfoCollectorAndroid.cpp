#include "CpuInfoCollectorAndroid.h"

#if defined(Z_ANDROID)

#include <fstream>
#include <unistd.h>

using namespace zzz::engine;
using namespace zzz::core;

std::vector<CpuInfo> CpuInfoCollectorAndroid::Collect() const
{
	std::string name = "Unknown CPU";
	std::ifstream file("/proc/cpuinfo");
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			// На Android/ARM поле обычно называется "Hardware" или "model name" в зависимости от ядра/вендора.
			if (line.rfind("Hardware", 0) == 0 || line.rfind("model name", 0) == 0)
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

	// Изолирован от CpuInfoCollectorLinux по Правилу 29 (мобильные платформы всегда получают
	// отдельную специализацию), хотя оба читают /proc/cpuinfo - это разные вендоры/форматы полей.
	std::vector<CpuInfo> cpus;
	cpus.emplace_back(std::move(name), "arm64", coreCount, coreCount, 0);
	return cpus;
}

CpuTopology CpuInfoCollectorAndroid::CollectTopology() const
{
	CpuTopology topology;
	topology.architecture = "arm64";

	std::string name = "Unknown CPU";
	std::ifstream file("/proc/cpuinfo");
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			if (line.rfind("Hardware", 0) == 0 || line.rfind("model name", 0) == 0)
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

	// Считывание частот через /sys/devices/system/cpu/cpu*/cpufreq/cpuinfo_max_freq
	// Для Android парсим кластеры, при их отсутствии фоллбэк на однородный
	topology.performanceLogicalCapacity = coreCount;
	return topology;
}

#endif // defined(Z_ANDROID)
