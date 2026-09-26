
#include <core/Core.h>
#include "AssetExtensions.h"
#include <core/io/storage/Path.h>
#include <core/constants/PackagesConstants.h>

#include "PackagePacker.h"
#include "AssetImporterRegistry.h"
#include "ProjectIdentityValidator.h"

#include "BuilderApi.h"

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
	BUILDER_API const char* GetGamePackageFileName()
	{
		return zzz::core::c_GamePackageFileName.c_str();
	}

	BUILDER_API const char* GetDataPackageFileName()
	{
		return zzz::core::c_DataPackageFileName.c_str();
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
}
