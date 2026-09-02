#include "RamInfoCollectorApple.h"

#if defined(Z_APPLE)

#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/vm_statistics.h>

using namespace zzz::engine;
using namespace zzz::core;

RamInfo RamInfoCollectorApple::Collect() const
{
	zU64 totalBytes = 1024ULL * 1024 * 1024; // честный фолбэк, гарантирует инвариант HardwareState (RAM > 0)
	std::size_t size = sizeof(totalBytes);
	sysctlbyname("hw.memsize", &totalBytes, &size, nullptr, 0);

	zU64 availableBytes = totalBytes / 2; // фолбэк, если host_statistics64 недоступен

	mach_port_t hostPort = mach_host_self();
	vm_size_t pageSize = 0;
	if (host_page_size(hostPort, &pageSize) == KERN_SUCCESS)
	{
		vm_statistics64_data_t vmStats{};
		mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
		if (host_statistics64(hostPort, HOST_VM_INFO64, reinterpret_cast<host_info64_t>(&vmStats), &count) == KERN_SUCCESS)
		{
			availableBytes = static_cast<zU64>(vmStats.free_count + vmStats.inactive_count) * pageSize;
		}
	}

	// Тип модулей ОЗУ недоступен через публичный sysctl/Mach API - оставлен Unknown.
	return RamInfo(totalBytes, availableBytes, eRamType::Unknown, 0);
}

#endif // defined(Z_APPLE)
