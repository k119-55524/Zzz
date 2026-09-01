#pragma once

#include "math/MathIncludes.h"

namespace zzz::math
{
	template<Arithmetic T = zI32>
	class Point2D;

	/**
	 * @class Size2D
	 * @brief Шаблонный класс для хранения и управления двумерными размерами (width, height).
	 *
	 * @tparam T Тип данных для хранения ширины и высоты (должен быть арифметическим).
	 *           Значение по умолчанию: zU32.
	 */
	template<Arithmetic T = zU32>
	class Size2D final
	{
	public:
		T width{ static_cast<T>(0) };
		T height{ static_cast<T>(0) };

		constexpr Size2D() noexcept = default;
		explicit constexpr Size2D(T size) noexcept : width{ size }, height{ size } {}
		constexpr Size2D(T inWidth, T inHeight) noexcept : width{ inWidth }, height{ inHeight } {}
		constexpr Size2D(const Size2D& size) noexcept : width{ size.width }, height{ size.height } {}
		constexpr Size2D(Size2D&&) noexcept = default;

		[[nodiscard]] constexpr const T* data() const noexcept { return &width; }
		[[nodiscard]] constexpr T* data() noexcept { return &width; }

		[[nodiscard]] constexpr const T& operator[](size_t index) const noexcept
		{
			assert(index < 2 && "Size2D index out of range");
			return (&width)[index];
		}

		[[nodiscard]] constexpr T& operator[](size_t index) noexcept
		{
			assert(index < 2 && "Size2D index out of range");
			return (&width)[index];
		}

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		inline void SetFrom(U w, U h) noexcept
		{
			width = static_cast<T>(w);
			height = static_cast<T>(h);
		}

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		inline void SetFrom(const Size2D<U>& other) noexcept
		{
			width = static_cast<T>(other.width);
			height = static_cast<T>(other.height);
		}

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		inline void SetFrom(const Point2D<U>& pt) noexcept;

		Size2D& operator=(const Size2D&) = default;
		Size2D& operator=(Size2D&&) noexcept = default;

		constexpr bool operator==(const Size2D&) const noexcept = default;

		constexpr Size2D operator+(const Size2D& other) const noexcept { return Size2D{ static_cast<T>(width + other.width), static_cast<T>(height + other.height) }; }
		constexpr Size2D operator-(const Size2D& other) const noexcept { return Size2D{ static_cast<T>(width - other.width), static_cast<T>(height - other.height) }; }

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		constexpr Size2D operator+(const Size2D<U>& other) const noexcept { return Size2D{ static_cast<T>(width + other.width), static_cast<T>(height + other.height) }; }

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		constexpr Size2D operator-(const Size2D<U>& other) const noexcept { return Size2D{ static_cast<T>(width - other.width), static_cast<T>(height - other.height) }; }

		constexpr Size2D& operator+=(const Size2D& other) noexcept { return *this = *this + other; }
		constexpr Size2D& operator-=(const Size2D& other) noexcept { return *this = *this - other; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Size2D operator*(S scale) const noexcept { return Size2D{ static_cast<T>(width * scale), static_cast<T>(height * scale) }; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Size2D operator/(S scale) const noexcept { return Size2D{ static_cast<T>(width / scale), static_cast<T>(height / scale) }; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Size2D& operator*=(S scale) noexcept { return *this = *this * scale; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Size2D& operator/=(S scale) noexcept { return *this = *this / scale; }

		[[nodiscard]] inline std::string ToString() const noexcept { return std::format("Width: {}, Height: {}", width, height); }
	};

	static_assert(std::is_standard_layout_v<Size2D<zU32>>, "Size2D must be standard layout");
	static_assert(sizeof(Size2D<zU32>) == 8, "Size2D<zU32> must be 8 bytes");
}

#include "math/Point2D.h"

namespace zzz::math
{
	template<Arithmetic T>
	template<Arithmetic U> requires SafelyConvertibleTo<U, T>
	inline void Size2D<T>::SetFrom(const Point2D<U>& pt) noexcept
	{
		width = static_cast<T>(pt.x);
		height = static_cast<T>(pt.y);
	}
}
