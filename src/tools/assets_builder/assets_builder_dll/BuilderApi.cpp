#include "BuilderApi.h"
#include <core/Core.h>
#include <core/io/storage/Path.h>
#include <core/constants/PackagesConstants.h>
#include "AssetExtensions.h"

#include "PackagePacker.h"
#include "AssetImporterRegistry.h"
#include "ProjectIdentityValidator.h"

namespace
{
	std::string NormalizeExt(const char* ext)
	{
		if (!ext) return {};
		std::string result(ext);
		std::ranges::transform(result, result.begin(), [](unsigned char ch) {
			return static_cast<char>(std::tolower(ch));
		});
		return result;
	}
}

extern "C"
{
	BUILDER_API const char* GetAssetsDirectoryName()
	{
		static const std::string s = zzz::core::c_AssetsDirectoryName.generic_string();
		return s.c_str();
	}

	BUILDER_API const char* GetGamePackageFileName()
	{
		return zzz::core::c_GamePackageFileName.c_str();
	}

	BUILDER_API const char* GetGamePackageRelativePath()
	{
		static const std::string s = zzz::core::c_GamePackageRelativePath.generic_string();
		return s.c_str();
	}

	BUILDER_API const uint8_t* GetGamePackageMagicBytes()
	{
		static const uint8_t magic[3] = {
			static_cast<uint8_t>(zzz::core::c_PackageDatFormat.Magic[0]),
			static_cast<uint8_t>(zzz::core::c_PackageDatFormat.Magic[1]),
			static_cast<uint8_t>(zzz::core::c_PackageDatFormat.Magic[2])
		};
		return magic;
	}

	BUILDER_API uint32_t GetGamePackageMajorVersion()
	{
		return zzz::core::c_PackageDatFormat.FormatVersion.GetMajor();
	}

	BUILDER_API uint32_t GetGamePackageMinorVersion()
	{
		return zzz::core::c_PackageDatFormat.FormatVersion.GetMinor();
	}

	BUILDER_API uint32_t GetGamePackagePatchVersion()
	{
		return zzz::core::c_PackageDatFormat.FormatVersion.GetPatch();
	}

	BUILDER_API uint32_t GetAssetTypeProjectManifest()
	{
		return static_cast<uint32_t>(zzz::core::ePackageDatType::ProjectManifest);
	}

	BUILDER_API uint32_t GetAssetTypeScene()
	{
		return static_cast<uint32_t>(zzz::core::ePackageDatType::Scene);
	}

	BUILDER_API uint32_t GetAssetTypePrimaryView()
	{
		return static_cast<uint32_t>(zzz::core::ePackageDatType::PrimaryView);
	}

	BUILDER_API uint32_t GetAssetTypeChildView()
	{
		return static_cast<uint32_t>(zzz::core::ePackageDatType::ChildView);
	}

	BUILDER_API uint32_t GetAssetTypeIndependentView()
	{
		return static_cast<uint32_t>(zzz::core::ePackageDatType::IndependentView);
	}

	BUILDER_API uint32_t GetAssetTypePrefab()
	{
		return static_cast<uint32_t>(zzz::core::ePackageDatType::Prefab);
	}

	BUILDER_API const char* GetDataPackageFileName()
	{
		return zzz::core::c_DataPackageFileName.c_str();
	}

	BUILDER_API const char* GetDataPackageRelativePath()
	{
		static const std::string s = zzz::core::c_DataPackageRelativePath.generic_string();
		return s.c_str();
	}

	BUILDER_API const uint8_t* GetDataPackageMagicBytes()
	{
		static const uint8_t magic[3] = {
			static_cast<uint8_t>(zzz::core::c_DataDatFormat.Magic[0]),
			static_cast<uint8_t>(zzz::core::c_DataDatFormat.Magic[1]),
			static_cast<uint8_t>(zzz::core::c_DataDatFormat.Magic[2])
		};
		return magic;
	}

	BUILDER_API uint32_t GetDataPackageMajorVersion()
	{
		return zzz::core::c_DataDatFormat.FormatVersion.GetMajor();
	}

	BUILDER_API uint32_t GetDataPackageMinorVersion()
	{
		return zzz::core::c_DataDatFormat.FormatVersion.GetMinor();
	}

	BUILDER_API uint32_t GetDataPackagePatchVersion()
	{
		return zzz::core::c_DataDatFormat.FormatVersion.GetPatch();
	}

	BUILDER_API bool PackProjectNative(const char* sourceDir, const char* destinationDir, uint32_t targetPlatform, const char* platformConfigFile, uint64_t inBuildTimestamp, uint64_t* outBuildTimestamp)
	{
		if (!sourceDir || !destinationDir) return false;
		std::string platformConfig = platformConfigFile ? platformConfigFile : "";
		return zzz::builder::PackagePacker::PackProject(
			sourceDir,
			destinationDir,
			static_cast<zzz::core::eTargetPlatform>(targetPlatform),
			platformConfig,
			inBuildTimestamp,
			outBuildTimestamp);
	}

	BUILDER_API bool ValidateDirectoryNameNative(const char* name)
	{
		if (!name) return false;
		return zzz::core::Path::IsValidDirectoryName(name);
	}


	BUILDER_API bool IsSupportedAssetExtension(const char* ext)
	{
		if (!ext) return false;
		const std::string norm = NormalizeExt(ext);
		return zzz::builder::AssetImporterRegistry::Instance().GetKnownType(norm).has_value();
	}

	BUILDER_API bool IsSupportedDataAssetExtension(const char* ext)
	{
		if (!ext) return false;
		const std::string norm = NormalizeExt(ext);
		return zzz::builder::AssetImporterRegistry::Instance().GetImporter(norm) != nullptr;
	}

	BUILDER_API bool IsSupportedViewExtension(const char* ext)
	{
		if (!ext) return false;
		const std::string norm = NormalizeExt(ext);
		auto known = zzz::builder::AssetImporterRegistry::Instance().GetKnownType(norm);
		return known.has_value() && *known == zzz::core::eEngineResourceType::View;
	}

	BUILDER_API bool GenerateGuidNative(char* outBuffer, uint32_t bufferSize)
	{
		if (!outBuffer || bufferSize < 37) return false;
		const auto str = zzz::core::Guid::Generate().ToString();
		std::memcpy(outBuffer, str.c_str(), 36);
		outBuffer[36] = '\0';
		return true;
	}

	BUILDER_API bool ValidateProjectIdentityNative(const char* projectDir, char* errorBuffer, uint32_t errorBufferSize, const char* platformConfigFile)
	{
		if (!projectDir)
		{
			if (errorBuffer && errorBufferSize > 0)
			{
				const char* msg = "Каталог проекта не задан (null).";
				const size_t len = std::min<size_t>(std::strlen(msg), errorBufferSize - 1);
				std::memcpy(errorBuffer, msg, len);
				errorBuffer[len] = '\0';
			}
			return false;
		}

		std::string cfg = platformConfigFile ? platformConfigFile : "";
		return zzz::builder::ProjectIdentityValidator::Validate(
			std::filesystem::path(reinterpret_cast<const char8_t*>(projectDir)),
			errorBuffer,
			errorBufferSize,
			cfg);
	}
}
