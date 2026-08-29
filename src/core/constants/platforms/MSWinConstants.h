#pragma once

/**
 * @file MSWinConstants.h
 * @brief Специфичные константы для платформы Microsoft Windows (Win32).
 *
 * @details Определяет идентификаторы встроенных ресурсов Win32 (например, имя
 *          ресурса иконки приложения в .rc файле) и специфичные константы платформы.
 *
 * @note Используется в:
 *       - MSWinWindow (загрузка иконки окна через LoadIcon/GetModuleHandle)
 *       - Platform / Native Application Handlers (Windows)
 */

#include "core/CoreIncludes.h"

namespace zzz::core
{
#pragma region Microsoft Windows constants
	/// Имя ресурса иконки приложения в скомпилированном Win32 .rc файле
	constexpr std::string_view c_IcoResourceName = "IDI_ICON1";
#pragma endregion // Microsoft Windows constants
}
