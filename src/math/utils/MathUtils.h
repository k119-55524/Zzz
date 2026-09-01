#pragma once

/**
 * @file MathUtils.h
 * @brief Вспомогательные математические функции модуля Math движка Zzz.
 *
 * @details Содержит constexpr-утилиты и хелперы для скалярных вычислений:
 *          - Вычисление модуля (Abs)
 *          - Получение точности и констант через функции (Epsilon, PI, HalfPI)
 *          - Преобразование единиц измерения углов (ToRadians, ToDegrees)
 *          - Базовые операции ограничения и интерполяции (Clamp, Lerp, Min, Max)
 */

#include "math/utils/Types.h"
#include "math/utils/MathConstants.h"
#include <algorithm>

namespace zzz::math::Math
{
	/// @brief Возвращает машинную погрешность сравнения для указанного типа T
	template<FloatingPoint T = zF32>
	[[nodiscard]] constexpr T Epsilon() noexcept
	{
		return Constants::Epsilon<T>;
	}

	/// @brief Возвращает число Пи для указанного типа T
	template<FloatingPoint T = zF32>
	[[nodiscard]] constexpr T PI() noexcept
	{
		return Constants::PI<T>;
	}

	/// @brief Возвращает число 2 * Пи для указанного типа T
	template<FloatingPoint T = zF32>
	[[nodiscard]] constexpr T TwoPI() noexcept
	{
		return Constants::TwoPI<T>;
	}

	/// @brief Возвращает число Пи / 2 для указанного типа T
	template<FloatingPoint T = zF32>
	[[nodiscard]] constexpr T HalfPI() noexcept
	{
		return Constants::HalfPI<T>;
	}

	/// @brief Возвращает коэффициент перевода градусов в радианы для типа T
	template<FloatingPoint T = zF32>
	[[nodiscard]] constexpr T Deg2Rad() noexcept
	{
		return Constants::Deg2Rad<T>;
	}

	/// @brief Возвращает коэффициент перевода радианов в градусы для типа T
	template<FloatingPoint T = zF32>
	[[nodiscard]] constexpr T Rad2Deg() noexcept
	{
		return Constants::Rad2Deg<T>;
	}

	/// @brief Переводит угол из градусов в радианы
	template<FloatingPoint T = zF32>
	[[nodiscard]] constexpr T ToRadians(T degrees) noexcept
	{
		return degrees * Constants::Deg2Rad<T>;
	}

	/// @brief Переводит угол из радианов в градусы
	template<FloatingPoint T = zF32>
	[[nodiscard]] constexpr T ToDegrees(T radians) noexcept
	{
		return radians * Constants::Rad2Deg<T>;
	}

	/// @brief Вычисляет абсолютное значение числа в constexpr-контексте
	template<typename T>
	[[nodiscard]] constexpr T Abs(T val) noexcept
	{
		return val < static_cast<T>(0) ? -val : val;
	}

	/// @brief Ограничивает значение в диапазоне [minVal, maxVal]
	template<typename T>
	[[nodiscard]] constexpr T Clamp(T val, T minVal, T maxVal) noexcept
	{
		return (val < minVal) ? minVal : ((val > maxVal) ? maxVal : val);
	}

	/// @brief Линейная интерполяция между двумя скалярными значениями a и b
	template<FloatingPoint T>
	[[nodiscard]] constexpr T Lerp(T a, T b, T t) noexcept
	{
		return a + (b - a) * t;
	}
}
