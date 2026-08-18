#pragma once

#include <cmath>
#include <format>
#include <string>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <expected>
#include <type_traits>
#include <string_view>
#include <concepts>

#include "math/Types.h"

namespace zzz::core
{
	class Serializer;
}

namespace zzz::math
{
	/// @brief Концепт для проверки арифметических типов
	template<typename T>
	concept Arithmetic = std::is_arithmetic_v<T>;

	/// @brief Концепт для безопасной конвертации типов без сужения разрядности
	template<typename From, typename To>
	concept SafelyConvertibleTo = std::convertible_to<From, To> && (sizeof(To) >= sizeof(From));
}
