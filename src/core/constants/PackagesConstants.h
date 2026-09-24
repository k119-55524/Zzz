#pragma once

/**
 * @file PackagesConstants.h
 * @brief Константы структуры хранения данных, пакетов ресурсов и файлов конфигурации.
 *
 * =========================================================================================
 * КАК ВРУЧНУЮ НАСТРОИТЬ СТРУКТУРУ ХРАНЕНИЯ ДАННЫХ ПРОЕКТА
 * =========================================================================================
 * Данные разделены на две независимые зоны (eFileLocation):
 *
 * 1. ЗОНА ДАННЫХ ПРИЛОЖЕНИЯ (Read-Only, каталог исполняемого файла, eFileLocation::App):
 *    - c_PackageName / c_DatExtension:
 *        Имя главного пакета ("package.dat").
 *        Хранит: манифест проекта, декларации представлений (Views) и сцены (SceneData).
 *    - c_GamePackageRelativePath:
 *        Расположение пакета ("assets/package.dat"). По умолчанию размещается внутри каталога
 *        ассетов c_AssetsDirectoryName, чтобы платформенные системы сборки (CMake/Android/iOS)
 *        копировали весь контент единой директорией assets.
 *    - c_AssetsDirectoryName:
 *        Имя каталога игровых ассетов ("assets").
 *    - c_DataPackageRelativePath:
 *        Расположение архива данных ("assets/data.dat").
 *        Хранит: таблицу оглавления (TOC) и полезную нагрузку ресурсов (меши, материалы, шейдеры).
 *    - Внешние пакеты ассетов {guid}.dat:
 *        Если ресурсы выносятся в отдельные файлы, они размещаются в каталоге
 *        c_AssetsDirectoryName рядом с data.dat ("assets/{guid}.dat").
 *
 * 2. ЗОНА ПОЛЬЗОВАТЕЛЯ (Read-Write, AppData / Home, eFileLocation::User):
 *    - c_UserConfigFileName ("cfg.dat"): файл настроек пользователя.
 *    - c_CacheDirectoryName ("cache"): временный кэш приложения.
 *    - c_SavesDirectoryName ("saves"): сохранения игрового процесса.
 *    - c_LogsDirectoryName  ("logs"): журналы работы приложения.
 * =========================================================================================
 */

#include <string>
#include <filesystem>
#include <string_view>

#include "core/utils/Version.h"

namespace zzz::core
{
#pragma region File and Path Names and Extensions
	inline constexpr std::string_view	c_DatExtension				= ".dat";
	inline constexpr std::string_view	c_PackageName				= "package";
	inline constexpr std::string_view	c_DataName					= "data";

	inline const std::filesystem::path	c_AssetsDirectoryName		= "assets";

	inline const std::string			c_GamePackageFileName		= std::string(c_PackageName) + std::string(c_DatExtension);
	inline const std::filesystem::path	c_GamePackageRelativePath	= c_AssetsDirectoryName / c_GamePackageFileName;
	inline const std::string			c_DataPackageFileName		= std::string(c_DataName) + std::string(c_DatExtension);
	inline const std::filesystem::path	c_DataPackageRelativePath	= c_AssetsDirectoryName / c_DataPackageFileName;

	inline constexpr std::string_view	c_UserConfigName			= "cfg";
	inline const std::string			c_UserConfigFileName		= std::string(c_UserConfigName) + std::string(c_DatExtension);

	inline constexpr std::string_view	c_CacheDirectoryName		= "cache";
	inline constexpr std::string_view	c_SavesDirectoryName		= "saves";
	inline constexpr std::string_view	c_LogsDirectoryName			= "logs";
#pragma endregion // File and Path Names and Extensions

#pragma region Dat File Formats (Signatures and Versions)
	using DatMagic = std::string_view;
	[[nodiscard]] consteval DatMagic operator""_magic(const char* str, std::size_t len) noexcept { return { str, len }; }
	struct DatFileFormat
	{
		DatMagic Magic;
		Version  FormatVersion;
	};

	inline constexpr DatFileFormat c_PackageDatFormat	{ "ZPD"_magic, Version(1, 0, 0) };
	inline constexpr DatFileFormat c_DataDatFormat		{ "ZDD"_magic, Version(1, 0, 0) };
	inline constexpr DatFileFormat c_UserConfigFormat	{ "ZUD"_magic, Version(1, 0, 0) };
	inline constexpr DatFileFormat c_AssetPackageFormat	{ "ZAP"_magic, Version(1, 0, 0) };
#pragma endregion
}