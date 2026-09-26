#include "BuilderApi.h"
#include <core/Core.h>
#include <core/io/storage/Path.h>
#include <core/constants/PackagesConstants.h>
#include "AssetExtensions.h"

#include "PackagePacker.h"
#include "AssetImporterRegistry.h"
#include "ProjectIdentityValidator.h"
#include "stages/validation/BuiltPackageValidator.h"


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

	BUILDER_API bool PackProjectNative(
		const char8_t* sourceDir,
		const char8_t* destinationDir,
		uint32_t targetPlatform,
		const char8_t* platformConfigFile,
		uint64_t inBuildTimestamp,
		uint64_t* outBuildTimestamp,
		char* errorBuffer,
		uint32_t errorBufferSize)
	{
		if (!sourceDir || !destinationDir)
		{
			if (errorBuffer && errorBufferSize > 0)
			{
				const char* msg = "Source or destination directory is null.";
				const size_t len = std::min<size_t>(std::strlen(msg), errorBufferSize - 1);
				std::memcpy(errorBuffer, msg, len);
				errorBuffer[len] = '\0';
			}
			return false;
		}

		std::filesystem::path sourcePath(sourceDir);
		std::filesystem::path destPath(destinationDir);
		std::string platformConfig = platformConfigFile ? reinterpret_cast<const char*>(platformConfigFile) : "";
		std::string errMsg;
		bool ok = zzz::builder::PackagePacker::PackProject(
			sourcePath,
			destPath,
			static_cast<zzz::core::eTargetPlatform>(targetPlatform),
			platformConfig,
			inBuildTimestamp,
			outBuildTimestamp,
			&errMsg);

		if (!ok && !errMsg.empty())
		{
			std::fprintf(stderr, "[PackProjectNative ERROR] %s\n", errMsg.c_str());
			if (errorBuffer && errorBufferSize > 0)
			{
				const size_t len = std::min<size_t>(errMsg.size(), errorBufferSize - 1);
				std::memcpy(errorBuffer, errMsg.data(), len);
				errorBuffer[len] = '\0';
			}
		}
		return ok;
	}

	BUILDER_API bool ValidateDirectoryNameNative(const char8_t* name)
	{
		if (!name) return false;
		return zzz::core::Path::IsValidDirectoryName(reinterpret_cast<const char*>(name));
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

	BUILDER_API bool ValidateProjectIdentityNative(const char8_t* projectDir, char* errorBuffer, uint32_t errorBufferSize, const char8_t* platformConfigFile)
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

		std::string cfg = platformConfigFile ? reinterpret_cast<const char*>(platformConfigFile) : "";
		return zzz::builder::ProjectIdentityValidator::Validate(
			std::filesystem::path(projectDir),
			errorBuffer,
			errorBufferSize,
			cfg);
	}

	BUILDER_API bool BeginBuildSessionNative(const char8_t* projectDir, char* errorBuffer, uint32_t errorBufferSize)
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

		std::string errorMsg;
		const bool success = zzz::builder::PackagePacker::BeginBuildSession(
			std::filesystem::path(projectDir),
			errorMsg);

		if (!success && errorBuffer && errorBufferSize > 0 && !errorMsg.empty())
		{
			const size_t len = std::min<size_t>(errorMsg.size(), errorBufferSize - 1);
			std::memcpy(errorBuffer, errorMsg.data(), len);
			errorBuffer[len] = '\0';
		}
		return success;
	}

	BUILDER_API void EndBuildSessionNative()
	{
		zzz::builder::PackagePacker::EndBuildSession();
	}

	BUILDER_API bool ValidateBuiltPackageNative(const char8_t* assetsDir, char* errorBuffer, uint32_t errorBufferSize)
	{
		if (!assetsDir)
		{
			if (errorBuffer && errorBufferSize > 0)
			{
				const char* msg = "Assets directory is null.";
				const size_t len = std::min<size_t>(std::strlen(msg), errorBufferSize - 1);
				std::memcpy(errorBuffer, msg, len);
				errorBuffer[len] = '\0';
			}
			return false;
		}

		auto report = zzz::builder::BuiltPackageValidator::Validate(
			std::filesystem::path(assetsDir));

		if (!report.isValid && errorBuffer && errorBufferSize > 0 && !report.errorMessage.empty())
		{
			const size_t len = std::min<size_t>(report.errorMessage.size(), errorBufferSize - 1);
			std::memcpy(errorBuffer, report.errorMessage.data(), len);
			errorBuffer[len] = '\0';
		}
		return report.isValid;
	}
}



