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

#include "core/CoreIncludes.h"
#include "core/io/FileHeader.h"

namespace zzz::core
{
#pragma region Config file constants
	/// Имя файла пользовательской конфигурации
	constexpr std::string_view c_ConfigFileName = "user.dat";

	/// Сигнатура (Magic Bytes) файла конфигурации: "ZZZ"
	constexpr FileHeader<3> c_ConfigHeader
	{
		std::array<std::byte, 3>{
			static_cast<std::byte>('Z'),
			static_cast<std::byte>('Z'),
			static_cast<std::byte>('Z')
		}
	};

	/// Мажорная версия формата файла конфигурации
	constexpr zU8 c_ConfigFileMajorVersion = 1;

	/// Минорная версия формата файла конфигурации (v1.1: добавлен opt-out список отключённых категорий логирования)
	constexpr zU8 c_ConfigFileMinorVersion = 1;

	/// Патч-версия формата файла конфигурации
	constexpr zU8 c_ConfigFilePatchVersion = 0;
#pragma endregion // Config file constants
}
