#include "StorageInfoCollectorMacOS.h"

#if defined(Z_MACOS)

#include <sys/mount.h>

using namespace zzz::engine;
using namespace zzz::core;

std::vector<StorageInfo> StorageInfoCollectorMacOS::Collect() const
{
	std::vector<StorageInfo> storages;

	struct statfs* mounts = nullptr;
	int count = getmntinfo(&mounts, MNT_NOWAIT);
	if (count <= 0 || !mounts)
		return storages;

	for (int i = 0; i < count; ++i)
	{
		const struct statfs& fs = mounts[i];

		std::string fsType = fs.f_fstypename;
		if (fsType == "devfs" || fsType == "autofs")
			continue;

		zU64 totalBytes = static_cast<zU64>(fs.f_blocks) * fs.f_bsize;
		zU64 freeBytes = static_cast<zU64>(fs.f_bfree) * fs.f_bsize;
		std::string mountPath = fs.f_mntonname;
		bool isSystemDrive = (mountPath == "/");

		// Тип накопителя (HDD/SSD/NVMe) на macOS требует запроса через DiskArbitration/IOKit -
		// отложено до этапа роллаута macOS, пока честно Unknown.
		storages.emplace_back(mountPath, mountPath, totalBytes, freeBytes, isSystemDrive, eStorageType::Unknown);
	}

	return storages;
}

#endif // defined(Z_MACOS)
