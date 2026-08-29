#pragma once

/**
 * @file DisplayConstants.h
 * @brief Константы оконной подсистемы и параметров отображения экрана.
 *
 * @details Содержит базовые параметры инициализации окон (дефолтные ширина и высота),
 *          ограничения минимальных размеров клиентской области для Desktop-платформ,
 *          а также стандартные пресеты разрешений дисплея (UHD 4K, 8K).
 *
 * @note Используется в:
 *       - WindowManager / Platform Window Creation (инициализация стартового окна)
 *       - ViewData (хранение дефолтных и минимальных размеров View)
 *       - Render / Viewport / Camera systems
 */

#include "core/CoreIncludes.h"

namespace zzz::core
{
#pragma region Display & Window constants
	/// Дефолтная ширина окна приложения в пикселях
	constexpr zU32 c_DefaultWindowWidth = 800;

	/// Дефолтная высота окна приложения в пикселях
	constexpr zU32 c_DefaultWindowHeight = 600;

	/// Дефолтный размер окна приложения (800x600)
	constexpr zzz::math::Size2D<zU32> c_DefaultWindowSize{ c_DefaultWindowWidth, c_DefaultWindowHeight };

#if Z_DESKTOP
	/// Минимальный размер окна (клиентской области) в пикселях для десктопных платформ
	constexpr zU32 c_MinWinSize = 150;
#endif // Z_DESKTOP

	/// Разрешение экрана Ultra HD 4K (3840x2160)
	constexpr zzz::math::Size2D<zU32> c_UHD_4K{ 3840, 2160 };

	/// Разрешение экрана Ultra HD 8K (7680x4320)
	constexpr zzz::math::Size2D<zU32> c_UHD_8K{ 7680, 4320 };
#pragma endregion // Display & Window constants
}
