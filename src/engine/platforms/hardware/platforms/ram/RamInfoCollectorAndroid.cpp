#include "RamInfoCollectorAndroid.h"

#if defined(Z_ANDROID)

#include <fstream>
#include <sstream>

using namespace zzz::engine;
using namespace zzz::core;

RamInfo RamInfoCollectorAndroid::Collect() const
{
	zU64 totalKb = 0;
	zU64 availKb = 0;

	std::ifstream file("/proc/meminfo");
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			std::istringstream iss(line);
			std::string key;
			zU64 value = 0;
			iss >> key >> value;

			if (key == "MemTotal:")
				totalKb = value;
			else if (key == "MemAvailable:")
				availKb = value;
		}
	}

	// Честный фолбэк на случай недоступности /proc/meminfo - гарантирует инвариант HardwareState (RAM > 0).
	if (totalKb == 0)
		totalKb = 1024 * 1024; // 1 GB

	return RamInfo(totalKb * 1024, availKb * 1024, eRamType::Unknown, 0);
}

#endif // defined(Z_ANDROID)
