#pragma once

/**
 * @file PackageConstants.h
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
 *    - c_UserConfigFileName ("cfg.dat", см. ConfigConstants.h): файл настроек пользователя.
 *    - c_CacheDirectoryName ("cache"): временный кэш приложения.
 *    - c_SavesDirectoryName ("saves"): сохранения игрового процесса.
 *    - c_LogsDirectoryName  ("logs"): журналы работы приложения.
 * =========================================================================================
 */

#include <array>
#include <string>
#include <cstddef>
#include <filesystem>
#include <string_view>

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

	inline constexpr std::string_view	c_CacheDirectoryName		= "cache";
	inline constexpr std::string_view	c_SavesDirectoryName		= "saves";
	inline constexpr std::string_view	c_LogsDirectoryName			= "logs";
#pragma endregion // File and Path Names and Extensions

#pragma region Game Data files constants
	/// Сигнатура package.dat
	constexpr std::array<std::byte, 3> c_PackageDatHeader
	{
		static_cast<std::byte>('Z'),
		static_cast<std::byte>('P'),
		static_cast<std::byte>('D')
	};

	constexpr zU8 c_PackageDatFileMajorVersion = 1;
	constexpr zU8 c_PackageDatFileMinorVersion = 0;
	constexpr zU8 c_PackageDatFilePatchVersion = 0;

	/// Сигнатура data.dat
	constexpr std::array<std::byte, 3> c_DataDatHeader
	{
		static_cast<std::byte>('Z'),
		static_cast<std::byte>('D'),
		static_cast<std::byte>('D')
	};

	constexpr zU8 c_DataDatFileMajorVersion = 1;
	constexpr zU8 c_DataDatFileMinorVersion = 0;
	constexpr zU8 c_DataDatFilePatchVersion = 0;

	/// Сигнатура внешних пакетов ассетов {guid}.dat
	constexpr std::array<std::byte, 3> c_AssetPackageHeader
	{
		static_cast<std::byte>('Z'),
		static_cast<std::byte>('A'),
		static_cast<std::byte>('P')
	};

	constexpr zU8 c_AssetPackageFileMajorVersion = 1;
	constexpr zU8 c_AssetPackageFileMinorVersion = 0;
	constexpr zU8 c_AssetPackageFilePatchVersion = 0;
#pragma endregion // Game Data files constants
}
