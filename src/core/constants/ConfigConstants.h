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
#include <string>
#include <string_view>
#include "core/constants/PackageConstants.h"
 
namespace zzz::core
{
#pragma region Config file constants
	inline constexpr std::string_view	c_UserConfigName		= "cfg";
	inline const std::string			c_UserConfigFileName	= std::string(c_UserConfigName) + std::string(c_DatExtension);

	/// Сигнатура файла конфигурации
	constexpr std::array<std::byte, 3> c_UserConfigHeader
	{
		static_cast<std::byte>('Z'),
		static_cast<std::byte>('U'),
		static_cast<std::byte>('D')
	};

	constexpr zU8 c_UserConfigFileMajorVersion = 1;
	constexpr zU8 c_UserConfigFileMinorVersion = 0;
	constexpr zU8 c_UserConfigFilePatchVersion = 0;
#pragma endregion // Config file constants
}
