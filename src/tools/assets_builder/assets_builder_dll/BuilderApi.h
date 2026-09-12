#pragma once

#include "BuilderExport.h"
#include <cstdint>

extern "C"
{
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

	/**
	 * @brief Упаковывает проект в package.dat/data.dat. Оба архива получают одно и то же время упаковки
	 * (см. DatFileHeader::GetBuildTime()).
	 * @param inBuildTimestamp Если > 0, используется как единый timestamp (мс от unix epoch) для архивов,
	 *        чтобы совпадать с buildtime-data.txt и assets_config.json. Если 0, генерируется автоматически.
	 * @param outBuildTimestamp Необязательный (может быть nullptr) выходной параметр - unix-время (мс),
	 *        которое было записано в заголовки package.dat/data.dat при успешной упаковке.
	 */
	BUILDER_API bool PackProjectNative(const char* sourceDir, const char* destinationDir, uint32_t targetPlatform, const char* platformConfigFile, uint64_t inBuildTimestamp, uint64_t* outBuildTimestamp);

	/**
	 * @brief Валидирует имя каталога (компании/приложения) по тем же правилам, что и Path::IsValidDirectoryName
	 * в движке (запрет Path Traversal, спецсимволов, зарезервированных имён Windows-устройств и т.д.) - единая
	 * логика валидации на стороне C++ (единственный источник истины), вызываемая из Assets Builder (C#) через P/Invoke.
	 * @param name Имя каталога в кодировке UTF-8 (на стороне C# - [MarshalAs(UnmanagedType.LPUTF8Str)]).
	 * @return true, если имя допустимо; false для nullptr, пустой строки или некорректного имени.
	 */
	BUILDER_API bool ValidateDirectoryNameNative(const char* name);

	/**
	 * @brief Проверяет, поддерживается ли указанное расширение ресурса сборщиком
	 * (на основе констант AssetExtensions.h и зарегистрированных импортеров AssetImporterRegistry).
	 * @param ext Расширение с точкой (например, ".obj", ".zs", ".zv").
	 * @return true, если тип ресурса поддерживается; false иначе.
	 */
	BUILDER_API bool IsSupportedAssetExtension(const char* ext);

	/**
	 * @brief Проверяет, относится ли расширение к ресурсам архива data.dat
	 * (Mesh, Material, Shader, Prefab - на основе констант AssetExtensions.h).
	 * @param ext Расширение с точкой (например, ".obj", ".zmat").
	 */
	BUILDER_API bool IsSupportedDataAssetExtension(const char* ext);

	/**
	 * @brief Проверяет, относится ли расширение к форматам окон (View -
	 * на основе констант AssetExtensions.h).
	 * @param ext Расширение с точкой (например, ".zv").
	 */
	BUILDER_API bool IsSupportedViewExtension(const char* ext);

	/**
	 * @brief Генерирует новый 128-битный UUID v4 через движковый генератор Guid::Generate()
	 * (Single Source of Truth) и форматирует в строку 36 символов (с завершающим нулём - 37 байт).
	 * @param outBuffer Выходной буфер символов.
	 * @param bufferSize Размер выходного буфера (должен быть не менее 37 байт).
	 * @return true при успешной записи; false если outBuffer == nullptr или bufferSize < 37.
	 */
	BUILDER_API bool GenerateGuidNative(char* outBuffer, uint32_t bufferSize);
}
