#pragma once

/**
 * @file AndroidConstants.h
 * @brief Специфичные константы для платформы Android.
 *
 * @details Содержит константы NativeActivity, AssetManager, JNI
 *          и параметры жизненного цикла приложения на Android.
 *
 * @note Используется в:
 *       - AndroidApp / Platform (Android)
 */

#include "core/utils/Defines.h"

#if defined(Z_ANDROID)

#include "core/CoreIncludes.h"

namespace zzz::core
{
#pragma region Android constants
	// Заготовка под специфичные константы платформы Android
#pragma endregion // Android constants
}

#endif // defined(Z_ANDROID)
