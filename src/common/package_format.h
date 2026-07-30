#pragma once

#include <array>
#include <string>
#include <common/version.h>
#include <common/constants.h>

namespace zzz::package
{
	enum class AssetType : zU32
	{
		ProjectManifest = 1,
		Scene = 2,
		View = 3,
		BinaryAsset = 4
	};

	using BinaryGuid = std::array<uint8_t, 16>;

	struct PackageHeader
	{
		std::array<std::byte, 3> magic = zzz::common::c_GamePackageHeader;
		zzz::common::Version version{
			zzz::common::c_GamePackageFileMajorVersion,
			zzz::common::c_GamePackageFileMinorVersion,
			zzz::common::c_GamePackageFilePatchVersion
		};
		zU32 entryCount = 0;
	};

	#pragma pack(push, 1)
	struct PackageEntry
	{
		BinaryGuid guid = {0};
		zU32 assetType = 0;
		zU64 offset = 0;
		zU64 size = 0;
	};
	#pragma pack(pop)

	struct ProjectManifestData
	{
		BinaryGuid gameScriptGuid = {0};
		std::vector<BinaryGuid> sceneGuids;
		std::vector<BinaryGuid> viewGuids;
	};

	struct SceneData
	{
		BinaryGuid sceneScriptGuid = {0};
	};

	struct ViewData
	{
		BinaryGuid sceneGuid = {0};
		std::vector<BinaryGuid> uiScriptGuids;
		zU32 elementsCount = 0;
	};
}
