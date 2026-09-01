#pragma once

#include <concepts>
#include <cassert>
#include "math/utils/Types.h"

namespace zzz::core
{
	/**
	 * @brief Выравнивает целочисленное значение вверх до ближайшего кратного alignment.
	 * @details Требует, чтобы alignment был степенью двойки (например, 4, 16, 256).
	 * @tparam T Целочисленный тип данных.
	 * @param value Исходное значение для выравнивания.
	 * @param alignment Граница выравнивания (должна быть степенью двойки > 0).
	 * @return Выровненное значение >= value.
	 */
	template<std::integral T>
	[[nodiscard]] constexpr T AlignUp(T value, T alignment) noexcept
	{
		assert((alignment > static_cast<T>(0)) && ((alignment & (alignment - static_cast<T>(1))) == static_cast<T>(0)) && "AlignUp: alignment must be a power of two");
		return (value + alignment - static_cast<T>(1)) & ~(alignment - static_cast<T>(1));
	}
}
