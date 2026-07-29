#include "BuilderApi.h"
#include <common/constants.h>
#include <common/package_format.h>

extern "C"
{
	BUILDER_API const char* GetBuilderEngineVersion()
	{
		return "1.0.0";
	}

	BUILDER_API const char* GetGamePackageFileName()
	{
		return zzz::common::c_GamePackageFileName.data();
	}

	BUILDER_API const uint8_t* GetGamePackageMagicBytes()
	{
		static const uint8_t magic[3] = {
			static_cast<uint8_t>(zzz::common::c_GamePackageHeader[0]),
			static_cast<uint8_t>(zzz::common::c_GamePackageHeader[1]),
			static_cast<uint8_t>(zzz::common::c_GamePackageHeader[2])
		};
		return magic;
	}

	BUILDER_API uint32_t GetGamePackageMajorVersion()
	{
		return zzz::common::c_GamePackageFileMajorVersion;
	}

	BUILDER_API uint32_t GetGamePackageMinorVersion()
	{
		return zzz::common::c_GamePackageFileMinorVersion;
	}

	BUILDER_API uint32_t GetGamePackagePatchVersion()
	{
		return zzz::common::c_GamePackageFilePatchVersion;
	}

	BUILDER_API uint32_t GetAssetTypeProjectManifest()
	{
		return static_cast<uint32_t>(zzz::package::AssetType::ProjectManifest);
	}

	BUILDER_API uint32_t GetAssetTypeScene()
	{
		return static_cast<uint32_t>(zzz::package::AssetType::Scene);
	}

	BUILDER_API uint32_t GetAssetTypeView()
	{
		return static_cast<uint32_t>(zzz::package::AssetType::View);
	}

	BUILDER_API uint32_t GetAssetTypeBinaryAsset()
	{
		return static_cast<uint32_t>(zzz::package::AssetType::BinaryAsset);
	}
}
