#pragma once

#include "math/MathIncludes.h"
#include "math/geometry/Size2D.h"

namespace zzz::math
{
	/**
	 * @class Point2D
	 * @brief Шаблонный класс для хранения и управления двумерными экранными координатами (X, Y).
	 *
	 * @tparam T Тип данных для хранения координат (должен быть арифметическим).
	 *           Значение по умолчанию: zI32.
	 */
	template<Arithmetic T = zI32>
	class Point2D final
	{
	public:
		T x{ static_cast<T>(0) };
		T y{ static_cast<T>(0) };

		constexpr Point2D() noexcept = default;
		explicit constexpr Point2D(T val) noexcept : x{ val }, y{ val } {}
		constexpr Point2D(T inX, T inY) noexcept : x{ inX }, y{ inY } {}
		constexpr Point2D(const Point2D& pt) noexcept : x{ pt.x }, y{ pt.y } {}
		constexpr Point2D(Point2D&&) noexcept = default;

		[[nodiscard]] constexpr const T* data() const noexcept { return &x; }
		[[nodiscard]] constexpr T* data() noexcept { return &x; }

		[[nodiscard]] constexpr const T& operator[](size_t index) const noexcept
		{
			assert(index < 2 && "Point2D index out of range");
			return (&x)[index];
		}

		[[nodiscard]] constexpr T& operator[](size_t index) noexcept
		{
			assert(index < 2 && "Point2D index out of range");
			return (&x)[index];
		}

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		inline void SetFrom(U inX, U inY) noexcept
		{
			x = static_cast<T>(inX);
			y = static_cast<T>(inY);
		}

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		inline void SetFrom(const Point2D<U>& other) noexcept
		{
			x = static_cast<T>(other.x);
			y = static_cast<T>(other.y);
		}

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		inline void SetFrom(const Size2D<U>& sz) noexcept
		{
			x = static_cast<T>(sz.width);
			y = static_cast<T>(sz.height);
		}

		Point2D& operator=(const Point2D&) = default;
		Point2D& operator=(Point2D&&) noexcept = default;

		constexpr bool operator==(const Point2D&) const noexcept = default;

		constexpr Point2D operator+(const Point2D& other) const noexcept { return Point2D{ static_cast<T>(x + other.x), static_cast<T>(y + other.y) }; }
		constexpr Point2D operator-(const Point2D& other) const noexcept { return Point2D{ static_cast<T>(x - other.x), static_cast<T>(y - other.y) }; }

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		constexpr Point2D operator+(const Size2D<U>& sz) const noexcept { return Point2D{ static_cast<T>(x + sz.width), static_cast<T>(y + sz.height) }; }

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		constexpr Point2D operator-(const Size2D<U>& sz) const noexcept { return Point2D{ static_cast<T>(x - sz.width), static_cast<T>(y - sz.height) }; }

		constexpr Point2D& operator+=(const Point2D& other) noexcept { return *this = *this + other; }
		constexpr Point2D& operator-=(const Point2D& other) noexcept { return *this = *this - other; }

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		constexpr Point2D& operator+=(const Size2D<U>& sz) noexcept { return *this = *this + sz; }

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		constexpr Point2D& operator-=(const Size2D<U>& sz) noexcept { return *this = *this - sz; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Point2D operator*(S scale) const noexcept { return Point2D{ static_cast<T>(x * scale), static_cast<T>(y * scale) }; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Point2D operator/(S scale) const noexcept { return Point2D{ static_cast<T>(x / scale), static_cast<T>(y / scale) }; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Point2D& operator*=(S scale) noexcept { return *this = *this * scale; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Point2D& operator/=(S scale) noexcept { return *this = *this / scale; }

		[[nodiscard]] inline zF64 DistanceTo(const Point2D& other) const noexcept
		{
			const zF64 dx = static_cast<zF64>(x - other.x);
			const zF64 dy = static_cast<zF64>(y - other.y);
			return std::sqrt(dx * dx + dy * dy);
		}

		inline void Offset(T dx, T dy) noexcept
		{
			x += dx;
			y += dy;
		}

		[[nodiscard]] inline std::string ToString() const noexcept { return std::format("X: {}, Y: {}", x, y); }
	};

	/// @brief Коммутативный оператор сложения: Size2D + Point2D -> Point2D
	template<Arithmetic T, Arithmetic U> requires SafelyConvertibleTo<U, T>
	constexpr Point2D<T> operator+(const Size2D<U>& sz, const Point2D<T>& pt) noexcept
	{
		return pt + sz;
	}

	static_assert(std::is_standard_layout_v<Point2D<zI32>>, "Point2D must be standard layout");
	static_assert(sizeof(Point2D<zI32>) == 8, "Point2D<zI32> must be 8 bytes");
}
