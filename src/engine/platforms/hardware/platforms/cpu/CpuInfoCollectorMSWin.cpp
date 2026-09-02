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

#endif // defined(Z_WINDOWS)
