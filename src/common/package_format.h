#pragma once

#include <array>
#include <common/common.h>
#include <common/constants.h>
#include <common/version.h>

namespace zzz::package
{
	enum class AssetType : zU32
	{
		ProjectManifest = 1,
		Scene = 2,
		View = 3,
		Script = 4,
		BinaryAsset = 5
	};

	#pragma pack(push, 1)
	struct PackageHeader
	{
		std::array<std::byte, 3> magic = zzz::common::c_GamePackageHeader;
		zzz::common::Version version
		{
			zzz::common::c_GamePackageFileMajorVersion,
			zzz::common::c_GamePackageFileMinorVersion,
			zzz::common::c_GamePackageFilePatchVersion
		};
		zU32 entryCount = 0;
	};

	struct PackageEntry
	{
		char guid[36] = {0};
		zU32 assetType = 0;
		zU64 offset = 0;
		zU64 size = 0;
	};
	#pragma pack(pop)
}
