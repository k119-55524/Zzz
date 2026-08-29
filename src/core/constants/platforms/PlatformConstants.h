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

#include "core/CoreIncludes.h"

#if Z_WINDOWS
#include "core/constants/platforms/MSWinConstants.h"
#elif Z_LINUX
#include "core/constants/platforms/LinuxConstants.h"
#elif Z_MACOS
#include "core/constants/platforms/MacOSConstants.h"
#elif Z_ANDROID
#include "core/constants/platforms/AndroidConstants.h"
#elif Z_IOS
#include "core/constants/platforms/iOSConstants.h"
#endif
