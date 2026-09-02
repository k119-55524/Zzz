#pragma once

/**
 * @file LinuxConstants.h
 * @brief Специфичные константы для платформы Linux.
 *
 * @details Содержит константы, параметры окружения и оконных серверов (X11 / Wayland),
 *          применяемые при сборке и работе под Linux.
 *
 * @note Используется в:
 *       - LinuxWindow / Platform (Linux)
 */

#include "core/utils/Defines.h"

#if defined(Z_LINUX)

#include "core/CoreIncludes.h"

namespace zzz::core
{
#pragma region Linux constants
	// Заготовка под специфичные константы платформы Linux
#pragma endregion // Linux constants
}

#endif // defined(Z_LINUX)
