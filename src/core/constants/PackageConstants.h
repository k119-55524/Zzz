#pragma once

/**
 * @file PackageConstants.h
 * @brief Константы формата игровых пакетов и архивов ресурсов (Game Package).
 *
 * @details Определяет относительный путь к файлу пакета по умолчанию, сигнатуру
 *          заголовка архива (Magic Bytes) и текущую версию формата пакета ресурсов.
 *
 * @note Используется в:
 *       - PackageManager (чтение, валидация и монтирование package.dat)
 *       - PackageHeader (сериализация/десериализация заголовка пакета)
 *       - PackageBuilder / Asset Pipeline (утилиты сборки и упаковки ресурсов)
 */

#include "core/CoreIncludes.h"
#include "core/io/FileHeader.h"

namespace zzz::core
{
#pragma region Game Package file constants
	/// Путь по умолчанию к основному архиву игровых ресурсов
	constexpr std::string_view c_GamePackageFileName = "assets/package.dat";

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
	constexpr zU8 c_GamePackageFileMinorVersion = 1;

	/// Патч-версия формата пакета ресурсов
	constexpr zU8 c_GamePackageFilePatchVersion = 0;
#pragma endregion // Game Package file constants
}
