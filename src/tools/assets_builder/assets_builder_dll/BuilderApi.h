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

	// Data Package constants (data.dat)
	BUILDER_API const char* GetDataPackageFileName();
	BUILDER_API const uint8_t* GetDataPackageMagicBytes();
	BUILDER_API uint32_t GetDataPackageMajorVersion();
	BUILDER_API uint32_t GetDataPackageMinorVersion();
	BUILDER_API uint32_t GetDataPackagePatchVersion();

	BUILDER_API bool PackProjectNative(const char* sourceDir, const char* destinationDir, uint32_t targetPlatform, const char* platformConfigFile);

	/**
	 * @brief Валидирует имя каталога (компании/приложения) по тем же правилам, что и Path::IsValidDirectoryName
	 * в движке (запрет Path Traversal, спецсимволов, зарезервированных имён Windows-устройств и т.д.) - единая
	 * логика валидации на стороне C++ (единственный источник истины), вызываемая из Assets Builder (C#) через P/Invoke.
	 * @param name Имя каталога в кодировке UTF-8 (на стороне C# - [MarshalAs(UnmanagedType.LPUTF8Str)]).
	 * @return true, если имя допустимо; false для nullptr, пустой строки или некорректного имени.
	 */
	BUILDER_API bool ValidateDirectoryNameNative(const char* name);

	/**
	 * @brief Проверяет, поддерживается ли указанное расширение ресурса движком Zzz Engine
	 * (на основе констант AssetFileExtensions.h и зарегистрированных импортеров AssetImporterRegistry).
	 * @param ext Расширение с точкой (например, ".obj", ".zs", ".zav").
	 * @return true, если тип ресурса поддерживается движком; false иначе.
	 */
	BUILDER_API bool IsSupportedAssetExtension(const char* ext);
}
