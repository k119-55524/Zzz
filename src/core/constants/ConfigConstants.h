#pragma once

/**
 * @file ConfigConstants.h
 * @brief Константы пользовательских и системных файлов конфигурации.
 *
 * @details Определяет имя основного файла конфигурации пользователя, сигнатуру
 *          заголовка (Magic Bytes) и текущую версию формата конфигурационного файла.
 *
 * @note Используется в:
 *       - UserSettingsManager (загрузка и сохранение настроек пользователя)
 *       - Configuration / Profile Serializer
 */

#include <array>
#include <cstddef>
#include "core/CoreIncludes.h"
 
namespace zzz::core
{
#pragma region Config file constants
	/// Имя файла пользовательской конфигурации
	constexpr std::string_view c_ConfigFileName = "user.dat";

	/// Сигнатура (Magic Bytes) файла конфигурации: "ZZZ"
	constexpr std::array<std::byte, 3> c_ConfigHeader
	{
		static_cast<std::byte>('Z'),
		static_cast<std::byte>('Z'),
		static_cast<std::byte>('Z')
	};

	/// Мажорная версия формата файла конфигурации
	constexpr zU8 c_ConfigFileMajorVersion = 1;

	/// Минорная версия формата файла конфигурации (v1.2: DatFileHeader с timestamp сохранения)
	constexpr zU8 c_ConfigFileMinorVersion = 2;

	/// Патч-версия формата файла конфигурации
	constexpr zU8 c_ConfigFilePatchVersion = 0;
#pragma endregion // Config file constants
}
