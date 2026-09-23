#pragma once

#include <limits>
#include <utility>
#include <optional>
#include <concepts>

namespace zzz::core
{
	/**
	 * @brief Безопасная арифметика размеров и диапазонов для разбора бинарных данных из файлов.
	 * @details Все значения, прочитанные из файла, считаются недоверенными. Функции ниже проверяют
	 *          переполнение и выход за границы до любой арифметики, приведения типов и выделения памяти.
	 */

	/// @brief Умножение без переполнения. Возвращает std::nullopt, если a * b не помещается в T.
	template <std::unsigned_integral T>
	[[nodiscard]] constexpr std::optional<T> CheckedMul(T a, T b) noexcept
	{
		if (a != 0 && b > std::numeric_limits<T>::max() / a)
			return std::nullopt;

		return static_cast<T>(a * b);
	}

	/// @brief Сложение без переполнения. Возвращает std::nullopt, если a + b не помещается в T.
	template <std::unsigned_integral T>
	[[nodiscard]] constexpr std::optional<T> CheckedAdd(T a, T b) noexcept
	{
		if (b > std::numeric_limits<T>::max() - a)
			return std::nullopt;

		return static_cast<T>(a + b);
	}

	/// @brief Проверяет, что диапазон [offset, offset + size) целиком лежит внутри [0, total).
	/// @details Не вычисляет offset + size, поэтому не переполняется.
	template <std::unsigned_integral T>
	[[nodiscard]] constexpr bool IsRangeInside(T offset, T size, T total) noexcept
	{
		return offset <= total && size <= total - offset;
	}

	/// @brief Сужающее преобразование без потери значения. Возвращает std::nullopt, если value не представимо в To.
	template <std::integral To, std::integral From>
	[[nodiscard]] constexpr std::optional<To> NarrowTo(From value) noexcept
	{
		if (!std::in_range<To>(value))
			return std::nullopt;

		return static_cast<To>(value);
	}
}
