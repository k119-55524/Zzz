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

#endif // defined(Z_ANDROID)
