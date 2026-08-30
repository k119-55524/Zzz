#pragma once

#include "BuilderExport.h"
#include <cstdint>

extern "C"
{
	BUILDER_API const char* GetBuilderEngineVersion();
	BUILDER_API const char* GetGamePackageFileName();
	BUILDER_API const uint8_t* GetGamePackageMagicBytes();
	BUILDER_API uint32_t GetGamePackageMajorVersion();
	BUILDER_API uint32_t GetGamePackageMinorVersion();
	BUILDER_API uint32_t GetGamePackagePatchVersion();

	BUILDER_API uint32_t GetAssetTypeProjectManifest();
	BUILDER_API uint32_t GetAssetTypeScene();
	BUILDER_API uint32_t GetAssetTypeChildView();
	BUILDER_API uint32_t GetAssetTypeIndependentView();
	BUILDER_API uint32_t GetAssetTypePrefab();
	BUILDER_API uint32_t GetAssetTypeBinaryAsset();
	BUILDER_API bool PackProjectNative(const char* sourceDir, const char* destinationDir, uint32_t targetPlatform);

	/**
	 * @brief Валидирует имя каталога (компании/приложения) по тем же правилам, что и Path::IsValidDirectoryName
	 * в движке (запрет Path Traversal, спецсимволов, зарезервированных имён Windows-устройств и т.д.) - единая
	 * логика валидации на стороне C++ (единственный источник истины), вызываемая из Assets Builder (C#) через P/Invoke.
	 * @param name Имя каталога в кодировке UTF-8 (на стороне C# - [MarshalAs(UnmanagedType.LPUTF8Str)]).
	 * @return true, если имя допустимо; false для nullptr, пустой строки или некорректного имени.
	 */
	BUILDER_API bool ValidateDirectoryNameNative(const char* name);
}
