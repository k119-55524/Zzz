#pragma once

/**
 * @file Constants.h
 * @brief Главный зонтичный (umbrella) заголовочный файл констант движка Zzz.
 *
 * @details Включает в себя все специализированные модули констант:
 *          - ConfigConstants.h       - Константы файлов пользовательской и системной конфигурации
 *          - DisplayConstants.h      - Размеры окон, ограничения и экранные разрешения
 *          - PackageConstants.h      - Константы архивов и пакетов ресурсов
 *          - GAPIConstants.h         - Параметры графических API (D3D12, Vulkan, Metal)
 *          - NetworkConstants.h      - Сетевые настройки и протокол удаленного логгера
 *          - LogCategoryConstants.h  - Встроенные категории логирования ядра
 *          - PlatformConstants.h     - Специфичные платформенные константы (Windows, Linux, Apple, Android)
 *
 * @note Используется в:
 *       - Core.h (подключение глобальных констант ядра)
 *       - Любых подсистемах движка, которым требуется доступ ко всем константам сразу
 */

#include "core/constants/ConfigConstants.h"
#include "core/constants/DisplayConstants.h"
#include "core/constants/PackageConstants.h"
#include "core/constants/GAPIConstants.h"
#include "core/constants/NetworkConstants.h"
#include "core/constants/LogCategoryConstants.h"
#include "core/constants/platforms/PlatformConstants.h"
