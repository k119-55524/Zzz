#pragma once

/**
 * @file MacOSConstants.h
 * @brief Специфичные константы для платформы macOS.
 *
 * @details Содержит константы структуры бандла (.app), ресурсов Cocoa/AppKit
 *          и параметры среды выполнения macOS.
 *
 * @note Используется в:
 *       - MacOSWindow / Platform (macOS)
 */

#include "core/utils/Defines.h"

#if defined(Z_MACOS)

#include "core/CoreIncludes.h"

namespace zzz::core
{
#pragma region MacOS constants
	// Заготовка под специфичные константы платформы macOS
#pragma endregion // MacOS constants
}

#endif // defined(Z_MACOS)
