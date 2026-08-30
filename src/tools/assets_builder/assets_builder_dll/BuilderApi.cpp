#include "BuilderApi.h"
#include <core/Core.h>
#include <core/io/Path.h>

#include "PackagePacker.h"

extern "C"
{
	BUILDER_API const char* GetBuilderEngineVersion()
	{
		return "1.0.0";
	}

	BUILDER_API const char* GetGamePackageFileName()
	{
		return zzz::core::c_GamePackageRelativePath.data();
	}

	BUILDER_API const uint8_t* GetGamePackageMagicBytes()
	{
		static const uint8_t magic[3] = {
			static_cast<uint8_t>(zzz::core::c_GamePackageHeader.GetMagic()[0]),
			static_cast<uint8_t>(zzz::core::c_GamePackageHeader.GetMagic()[1]),
			static_cast<uint8_t>(zzz::core::c_GamePackageHeader.GetMagic()[2])
		};
		return magic;
	}

	BUILDER_API uint32_t GetGamePackageMajorVersion()
	{
		return zzz::core::c_GamePackageFileMajorVersion;
	}

	BUILDER_API uint32_t GetGamePackageMinorVersion()
	{
		return zzz::core::c_GamePackageFileMinorVersion;
	}

	BUILDER_API uint32_t GetGamePackagePatchVersion()
	{
		return zzz::core::c_GamePackageFilePatchVersion;
	}

	BUILDER_API uint32_t GetAssetTypeProjectManifest()
	{
		return static_cast<uint32_t>(zzz::core::ePackage::ProjectManifest);
	}

	BUILDER_API uint32_t GetAssetTypeScene()
	{
		return static_cast<uint32_t>(zzz::core::ePackage::Scene);
	}

	BUILDER_API uint32_t GetAssetTypeChildView()
	{
		return static_cast<uint32_t>(zzz::core::ePackage::ChildView);
	}

	BUILDER_API uint32_t GetAssetTypeIndependentView()
	{
		return static_cast<uint32_t>(zzz::core::ePackage::IndependentView);
	}

	BUILDER_API uint32_t GetAssetTypePrefab()
	{
		return static_cast<uint32_t>(zzz::core::ePackage::Prefab);
	}

	BUILDER_API uint32_t GetAssetTypeBinaryAsset()
	{
		return static_cast<uint32_t>(zzz::core::ePackage::BinaryAsset);
	}

	BUILDER_API bool PackProjectNative(const char* sourceDir, const char* destinationDir, uint32_t targetPlatform)
	{
		if (!sourceDir || !destinationDir) return false;
		return zzz::builder::PackagePacker::PackProject(
			sourceDir,
			destinationDir,
			static_cast<zzz::core::eTargetPlatform>(targetPlatform));
	}

	BUILDER_API bool ValidateDirectoryNameNative(const char* name)
	{
		if (!name) return false;
		return zzz::core::Path::IsValidDirectoryName(name);
	}
}
