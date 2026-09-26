#pragma once

#include <cstdint>

#include "BuilderExport.h"

extern "C"
{
	BUILDER_API const char* GetGamePackageFileName();
	BUILDER_API const char* GetDataPackageFileName();

	/**
	 * @brief Упаковывает проект в package.dat/data.dat. Оба архива получают одно и то же время упаковки
	 * (см. DatFileHeader::GetTimestamp()).
	 * @param sourceDir Каталог проекта в UTF-8 (char8_t).
	 * @param destinationDir Каталог назначения в UTF-8 (char8_t).
	 * @param inBuildTimestamp Если > 0, используется как единый timestamp (мс от unix epoch) для архивов,
	 *        чтобы совпадать с buildtime-data.txt и assets_config.json. Если 0, генерируется автоматически.
	 * @param outBuildTimestamp Необязательный (может быть nullptr) выходной параметр - unix-время (мс),
	 *        которое было записано в заголовки package.dat/data.dat при успешной упаковке.
	 * @param errorBuffer Выделенный вызывающей стороной буфер под текст ошибки.
	 * @param errorBufferSize Размер буфера ошибки.
	 */
	BUILDER_API bool PackProjectNative(
		const char8_t* sourceDir,
		const char8_t* destinationDir,
		uint32_t targetPlatform,
		const char8_t* platformConfigFile,
		uint64_t inBuildTimestamp,
		uint64_t* outBuildTimestamp,
		char* errorBuffer = nullptr,
		uint32_t errorBufferSize = 0);

	/**
	 * @brief Валидирует имя каталога (компании/приложения) по тем же правилам, что и Path::IsValidDirectoryName
	 * в движке (запрет Path Traversal, спецсимволов, зарезервированных имён Windows-устройств и т.д.) - единая
	 * логика валидации на стороне C++ (единственный источник истины), вызываемая из Assets Builder (C#) через P/Invoke.
	 * @param name Имя каталога в кодировке UTF-8 (на стороне C# - [MarshalAs(UnmanagedType.LPUTF8Str)]).
	 * @return true, если имя допустимо; false для nullptr, пустой строки или некорректного имени.
	 */
	BUILDER_API bool ValidateDirectoryNameNative(const char8_t* name);

	/**
	 * @brief Проверяет, поддерживается ли указанное расширение ресурса сборщиком
	 * (на основе констант AssetExtensions.h и зарегистрированных импортеров AssetImporterRegistry).
	 * @param ext Расширение с точкой (например, ".obj", ".zscene", ".zview").
	 * @return true, если тип ресурса поддерживается; false иначе.
	 */
	BUILDER_API bool IsSupportedAssetExtension(const char* ext);

	/**
	 * @brief Проверяет, относится ли расширение к ресурсам архива data.dat
	 * (Mesh, Material, Shader, Prefab - на основе констант AssetExtensions.h).
	 * @param ext Расширение с точкой (например, ".obj", ".zmaterial").
	 */
	BUILDER_API bool IsSupportedDataAssetExtension(const char* ext);

	/**
	 * @brief Проверяет, относится ли расширение к форматам окон (View -
	 * на основе констант AssetExtensions.h).
	 * @param ext Расширение с точкой (например, ".zview").
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

	/**
	 * @brief Проверяет глобальную уникальность GUID и ссылочную целостность проекта (нативный Single Source of Truth).
	 * @param projectDir Корневой каталог проекта в UTF-8.
	 * @param errorBuffer Выделенный вызывающей стороной буфер под текст ошибки в кодировке UTF-8.
	 * @param errorBufferSize Размер буфера ошибки.
	 * @param platformConfigFile Опциональный путь к конкретному файлу платформенной конфигурации.
	 * @return true, если проект валиден; false, если обнаружена ошибка (сообщение записывается в errorBuffer).
	 */
	BUILDER_API bool ValidateProjectIdentityNative(const char8_t* projectDir, char* errorBuffer, uint32_t errorBufferSize, const char8_t* platformConfigFile = nullptr);

	/**
	 * @brief Начинает сессию сборки проекта. Выполняет Стадию 1 (валидация проекта) один раз и кэширует её.
	 * @param projectDir Корневой каталог проекта в UTF-8.
	 * @param errorBuffer Выделенный вызывающей стороной буфер под текст ошибки.
	 * @param errorBufferSize Размер буфера ошибки.
	 * @return true при успехе; false при ошибке валидации.
	 */
	BUILDER_API bool BeginBuildSessionNative(const char8_t* projectDir, char* errorBuffer, uint32_t errorBufferSize);

	/**
	 * @brief Завершает сессию сборки проекта и сбрасывает кэш сессии.
	 */
	BUILDER_API void EndBuildSessionNative();
}
