#pragma once

/**
 * @file MathConstants.h
 * @brief Фундаментальные математические константы модуля Math движка Zzz.
 *
 * @details Содержит типизированные константы точности с плавающей запятой:
 *          - Эпсилон погрешности сравнения чисел (Epsilon для float и double)
 *          - Число Пи (PI, 2*PI, PI/2)
 *          - Коэффициенты преобразования градусов и радианов (Deg2Rad, Rad2Deg)
 */

#include "math/utils/Types.h"
#include <concepts>
#include <type_traits>

namespace zzz::math
{
	/// @brief Концепт для проверки вещественных типов с плавающей запятой
	template<typename T>
	concept FloatingPoint = std::floating_point<T>;

	namespace Constants
	{
		/// @brief Машинная точность / эпсилон для операций сравнения вещественных чисел
		template<FloatingPoint T>
		inline constexpr T Epsilon = std::is_same_v<T, zF32> ? static_cast<T>(1e-5) : static_cast<T>(1e-9);

		/// @brief Число Пи (3.14159265358979323846...)
		template<FloatingPoint T>
		inline constexpr T PI = static_cast<T>(3.141592653589793238462643383279502884);

		/// @brief Число 2 * Пи (6.28318530717958647692...)
		template<FloatingPoint T>
		inline constexpr T TwoPI = static_cast<T>(6.283185307179586476925286766559005768);

		/// @brief Число Пи / 2 (1.57079632679489661923...)
		template<FloatingPoint T>
		inline constexpr T HalfPI = static_cast<T>(1.570796326794896619231321691639751442);

		/// @brief Коэффициент перевода градусов в радианы (PI / 180)
		template<FloatingPoint T>
		inline constexpr T Deg2Rad = PI<T> / static_cast<T>(180);

		/// @brief Коэффициент перевода радианов в градусы (180 / PI)
		template<FloatingPoint T>
		inline constexpr T Rad2Deg = static_cast<T>(180) / PI<T>;
	}
}
