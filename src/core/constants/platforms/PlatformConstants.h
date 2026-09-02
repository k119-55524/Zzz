#pragma once

/**
 * @file PlatformConstants.h
 * @brief Агрегирующий заголовок специфичных констант целевой платформы.
 *
 * @details Автоматически подключает соответствующий заголовок платформы
 *          (MSWinConstants.h, LinuxConstants.h, MacOSConstants.h, AndroidConstants.h, iOSConstants.h)
 *          на основе макросов конфигурации сборки Z_WINDOWS, Z_LINUX, Z_MACOS, Z_ANDROID, Z_IOS.
 *
 * @note Используется в:
 *       - Платформенных подсистемах и оконных менеджерах
 *       - Общем зонтичном заголовке core/constants/Constants.h
 */

#include "core/utils/Defines.h"

#if defined(Z_WINDOWS)
#include "core/constants/platforms/MSWinConstants.h"
#elif defined(Z_LINUX)
#include "core/constants/platforms/LinuxConstants.h"
#elif defined(Z_MACOS)
#include "core/constants/platforms/MacOSConstants.h"
#elif defined(Z_ANDROID)
#include "core/constants/platforms/AndroidConstants.h"
#elif defined(Z_IOS)
#include "core/constants/platforms/iOSConstants.h"
#else
#error ">>>>> PlatformConstants: Unsupported platform."
#endif
