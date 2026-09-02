#include "StorageInfoCollectorLinux.h"

#if defined(Z_LINUX)

#include <fstream>
#include <sstream>
#include <unordered_set>
#include <sys/statvfs.h>

using namespace zzz::engine;
using namespace zzz::core;

namespace
{
	bool IsPseudoFilesystem(const std::string& fsType)
	{
		static const std::unordered_set<std::string> pseudo = {
			"proc", "sysfs", "devtmpfs", "tmpfs", "devpts", "cgroup", "cgroup2",
			"pstore", "bpf", "tracefs", "debugfs", "mqueue", "hugetlbfs", "securityfs",
			"autofs", "configfs", "fusectl", "binfmt_misc", "overlay", "squashfs", "efivarfs"
		};
		return pseudo.contains(fsType);
	}
}

std::vector<StorageInfo> StorageInfoCollectorLinux::Collect() const
{
	std::vector<StorageInfo> storages;

	std::ifstream file("/proc/mounts");
	if (!file.is_open())
		return storages;

	std::string line;
	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		std::string device, mountPoint, fsType;
		iss >> device >> mountPoint >> fsType;

		if (mountPoint.empty() || IsPseudoFilesystem(fsType))
			continue;

		struct statvfs vfs{};
		if (statvfs(mountPoint.c_str(), &vfs) != 0)
			continue;

		zU64 totalBytes = static_cast<zU64>(vfs.f_blocks) * vfs.f_frsize;
		zU64 freeBytes = static_cast<zU64>(vfs.f_bfree) * vfs.f_frsize;
		bool isSystemDrive = (mountPoint == "/");

		// Тип накопителя (HDD/SSD/NVMe) на Linux требует чтения /sys/block/<dev>/queue/rotational -
		// отложено до этапа роллаута Linux, пока честно Unknown.
		storages.emplace_back(mountPoint, mountPoint, totalBytes, freeBytes, isSystemDrive, eStorageType::Unknown);
	}

	return storages;
}

#endif // defined(Z_LINUX)
