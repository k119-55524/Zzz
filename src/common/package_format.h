#pragma once

#include <array>
#include <string>
#include <common/guid.h>
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

	using BinaryGuid = zzz::common::Guid;

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
		BinaryGuid guid = {};
		zU32 assetType = 0;
		zU64 offset = 0;
		zU64 size = 0;
	};
	#pragma pack(pop)

	struct ProjectManifestData
	{
		BinaryGuid gameScriptGuid = {};
		std::vector<BinaryGuid> sceneGuids;
		std::vector<BinaryGuid> viewGuids;
	};

	struct SceneData
	{
		BinaryGuid sceneScriptGuid = {};
	};

	struct ViewData
	{
		BinaryGuid sceneGuid = {};
		std::string name;
		std::vector<BinaryGuid> uiScriptGuids;
	};
}
