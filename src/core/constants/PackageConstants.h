#pragma once

#include "core/CoreIncludes.h"
#include "core/io/FileHeader.h"

namespace zzz::core
{
#pragma region Path composition constants
	/// Имя каталога игровых ассетов (Read-Only, поставляется с игрой) - используется и напрямую, и в путях ниже.
	inline constexpr char c_AssetsDirectoryName[] = "assets";

	/// Максимальная длина имени ассета в символах (кодовых точках Unicode, не байтах) в таблице пакетов (PackageEntry, см. core::FixedLengthString32)
	inline constexpr std::size_t c_MaxAssetNameLength = 64;

	/// Относительные (от каталога исполняемого файла) пути к файлам/каталогам ассетов.
	inline constexpr std::string_view c_GamePackageRelativePath       = "assets/package.dat";
	inline constexpr std::string_view c_PaksDirectoryRelativePath     = "assets/paks";
	inline constexpr std::string_view c_DataDirectoryRelativePath     = "assets/data";
	inline constexpr std::string_view c_DataPackageRelativePath       = "assets/data/data.dat";
	inline constexpr std::string_view c_TexturesDirectoryRelativePath = "assets/data/textures";
	inline constexpr std::string_view c_VideoDirectoryRelativePath    = "assets/data/video";
	inline constexpr std::string_view c_AudioDirectoryRelativePath    = "assets/data/audio";
	inline constexpr std::string_view c_FontsDirectoryRelativePath    = "assets/data/fonts";

	/// Имена подкаталогов внутри каталога пользовательских данных (Read-Write, машина пользователя).
	inline constexpr std::string_view c_CacheDirectoryName = "cache";
	inline constexpr std::string_view c_SavesDirectoryName = "saves";
	inline constexpr std::string_view c_LogsDirectoryName = "logs";
#pragma endregion // Path composition constants

#pragma region Game Package file constants
	/// Сигнатура (Magic Bytes) файла пакета ресурсов: "ZZP"
	constexpr FileHeader<3> c_GamePackageHeader
	{
		std::array<std::byte, 3>{
			static_cast<std::byte>('Z'),
			static_cast<std::byte>('Z'),
			static_cast<std::byte>('P')
		}
	};

	/// Мажорная версия формата пакета ресурсов
	constexpr zU8 c_GamePackageFileMajorVersion = 1;

	/// Минорная версия формата пакета ресурсов
	constexpr zU8 c_GamePackageFileMinorVersion = 0;

	/// Патч-версия формата пакета ресурсов
	constexpr zU8 c_GamePackageFilePatchVersion = 0;
#pragma endregion // Game Package file constants

#pragma region Data Package file constants
	/// Сигнатура (Magic Bytes) файла пакета данных: "ZZD"
	constexpr FileHeader<3> c_DataPackageHeader
	{
		std::array<std::byte, 3>{
			static_cast<std::byte>('Z'),
			static_cast<std::byte>('Z'),
			static_cast<std::byte>('D')
		}
	};

	/// Мажорная версия формата пакета данных
	constexpr zU8 c_DataPackageFileMajorVersion = 1;

	/// Минорная версия формата пакета данных
	constexpr zU8 c_DataPackageFileMinorVersion = 0;

	/// Патч-версия формата пакета данных
	constexpr zU8 c_DataPackageFilePatchVersion = 0;
#pragma endregion // Data Package file constants

#pragma region Scene and GameObject JSON constants
	inline constexpr std::string_view c_FieldDomain       = "domain";
	inline constexpr std::string_view c_DomainObject      = "Object";
	inline constexpr std::string_view c_DomainEntity      = "Entity";
#pragma endregion // Scene and GameObject JSON constants
}
