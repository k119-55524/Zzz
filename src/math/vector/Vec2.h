#pragma once

#include "math/MathIncludes.h"

namespace zzz::math
{
	/**
	 * @struct Vec2
	 * @brief Шаблонная структура двумерного вектора (X, Y).
	 * @tparam T Арифметический тип данных (по умолчанию zF32).
	 */
	template<Arithmetic T = zF32>
	struct Vec2
	{
		T x{ static_cast<T>(0) };
		T y{ static_cast<T>(0) };

		constexpr Vec2() noexcept = default;
		explicit constexpr Vec2(T scalar) noexcept : x{ scalar }, y{ scalar } {}
		constexpr Vec2(T inX, T inY) noexcept : x{ inX }, y{ inY } {}

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		constexpr Vec2(const Vec2<U>& other) noexcept
			: x{ static_cast<T>(other.x) }
			, y{ static_cast<T>(other.y) }
		{
		}

		[[nodiscard]] constexpr const T* data() const noexcept { return &x; }
		[[nodiscard]] constexpr T* data() noexcept { return &x; }

		[[nodiscard]] constexpr const T& operator[](size_t index) const noexcept
		{
			assert(index < 2 && "Vec2 index out of range");
			return (&x)[index];
		}

		[[nodiscard]] constexpr T& operator[](size_t index) noexcept
		{
			assert(index < 2 && "Vec2 index out of range");
			return (&x)[index];
		}

		[[nodiscard]] constexpr Vec2 operator+(const Vec2& other) const noexcept
		{
			return Vec2{ static_cast<T>(x + other.x), static_cast<T>(y + other.y) };
		}

		[[nodiscard]] constexpr Vec2 operator-(const Vec2& other) const noexcept
		{
			return Vec2{ static_cast<T>(x - other.x), static_cast<T>(y - other.y) };
		}

		[[nodiscard]] constexpr Vec2 operator*(const Vec2& other) const noexcept
		{
			return Vec2{ static_cast<T>(x * other.x), static_cast<T>(y * other.y) };
		}

		[[nodiscard]] constexpr Vec2 operator/(const Vec2& other) const noexcept
		{
			return Vec2{ static_cast<T>(x / other.x), static_cast<T>(y / other.y) };
		}

		template<Arithmetic S>
		[[nodiscard]] constexpr Vec2 operator*(S scalar) const noexcept
		{
			return Vec2{ static_cast<T>(x * scalar), static_cast<T>(y * scalar) };
		}

		template<Arithmetic S>
		[[nodiscard]] constexpr Vec2 operator/(S scalar) const noexcept
		{
			return Vec2{ static_cast<T>(x / scalar), static_cast<T>(y / scalar) };
		}

		[[nodiscard]] constexpr Vec2 operator-() const noexcept
		{
			return Vec2{ static_cast<T>(-x), static_cast<T>(-y) };
		}

		constexpr Vec2& operator+=(const Vec2& other) noexcept
		{
			x = static_cast<T>(x + other.x);
			y = static_cast<T>(y + other.y);
			return *this;
		}

		constexpr Vec2& operator-=(const Vec2& other) noexcept
		{
			x = static_cast<T>(x - other.x);
			y = static_cast<T>(y - other.y);
			return *this;
		}

		constexpr Vec2& operator*=(const Vec2& other) noexcept
		{
			x = static_cast<T>(x * other.x);
			y = static_cast<T>(y * other.y);
			return *this;
		}

		constexpr Vec2& operator/=(const Vec2& other) noexcept
		{
			x = static_cast<T>(x / other.x);
			y = static_cast<T>(y / other.y);
			return *this;
		}

		template<Arithmetic S>
		constexpr Vec2& operator*=(S scalar) noexcept
		{
			x = static_cast<T>(x * scalar);
			y = static_cast<T>(y * scalar);
			return *this;
		}

		template<Arithmetic S>
		constexpr Vec2& operator/=(S scalar) noexcept
		{
			x = static_cast<T>(x / scalar);
			y = static_cast<T>(y / scalar);
			return *this;
		}

		[[nodiscard]] constexpr bool operator==(const Vec2& other) const noexcept
		{
			if constexpr (std::floating_point<T>)
			{
				constexpr T eps = static_cast<T>(1e-5);
				return std::abs(x - other.x) <= eps && std::abs(y - other.y) <= eps;
			}
			else
			{
				return x == other.x && y == other.y;
			}
		}

		[[nodiscard]] constexpr bool operator!=(const Vec2& other) const noexcept
		{
			return !(*this == other);
		}

		[[nodiscard]] constexpr T Dot(const Vec2& other) const noexcept
		{
			return static_cast<T>(x * other.x + y * other.y);
		}

		[[nodiscard]] constexpr T LengthSquared() const noexcept
		{
			return Dot(*this);
		}

		[[nodiscard]] auto Length() const noexcept
		{
			return std::sqrt(LengthSquared());
		}

		Vec2& Normalize() noexcept requires std::floating_point<T>
		{
			const T len = Length();
			if (len > static_cast<T>(1e-6))
			{
				x /= len;
				y /= len;
			}
			else
			{
				x = static_cast<T>(0);
				y = static_cast<T>(0);
			}
			return *this;
		}

		[[nodiscard]] Vec2 Normalized() const noexcept requires std::floating_point<T>
		{
			Vec2 result = *this;
			return result.Normalize();
		}

		[[nodiscard]] auto Distance(const Vec2& other) const noexcept
		{
			return (*this - other).Length();
		}

		[[nodiscard]] constexpr Vec2 Lerp(const Vec2& target, zF32 t) const noexcept
		{
			return Vec2{
				static_cast<T>(x + (target.x - x) * t),
				static_cast<T>(y + (target.y - y) * t)
			};
		}

		[[nodiscard]] Vec2 Reflect(const Vec2& normal) const noexcept requires std::floating_point<T>
		{
			return *this - normal * (static_cast<T>(2) * Dot(normal));
		}

		[[nodiscard]] std::string ToString() const
		{
			return std::format("Vec2({}, {})", x, y);
		}

		[[nodiscard]] static constexpr Vec2 Zero() noexcept { return Vec2{ static_cast<T>(0), static_cast<T>(0) }; }
		[[nodiscard]] static constexpr Vec2 One() noexcept { return Vec2{ static_cast<T>(1), static_cast<T>(1) }; }
		[[nodiscard]] static constexpr Vec2 UnitX() noexcept { return Vec2{ static_cast<T>(1), static_cast<T>(0) }; }
		[[nodiscard]] static constexpr Vec2 UnitY() noexcept { return Vec2{ static_cast<T>(0), static_cast<T>(1) }; }
	};

	template<Arithmetic S, Arithmetic T>
	[[nodiscard]] constexpr Vec2<T> operator*(S scalar, const Vec2<T>& vec) noexcept
	{
		return vec * scalar;
	}

	template<Arithmetic T>
	[[nodiscard]] constexpr T Dot(const Vec2<T>& a, const Vec2<T>& b) noexcept
	{
		return a.Dot(b);
	}

	template<Arithmetic T>
	[[nodiscard]] auto Distance(const Vec2<T>& a, const Vec2<T>& b) noexcept
	{
		return a.Distance(b);
	}

	template<Arithmetic T>
	[[nodiscard]] constexpr Vec2<T> Lerp(const Vec2<T>& a, const Vec2<T>& b, zF32 t) noexcept
	{
		return a.Lerp(b, t);
	}

	template<Arithmetic T>
	[[nodiscard]] constexpr Vec2<T> Min(const Vec2<T>& a, const Vec2<T>& b) noexcept
	{
		return Vec2<T>{ std::min(a.x, b.x), std::min(a.y, b.y) };
	}

	template<Arithmetic T>
	[[nodiscard]] constexpr Vec2<T> Max(const Vec2<T>& a, const Vec2<T>& b) noexcept
	{
		return Vec2<T>{ std::max(a.x, b.x), std::max(a.y, b.y) };
	}

	template<Arithmetic T>
	[[nodiscard]] constexpr Vec2<T> Clamp(const Vec2<T>& v, const Vec2<T>& minVal, const Vec2<T>& maxVal) noexcept
	{
		return Vec2<T>{
			std::clamp(v.x, minVal.x, maxVal.x),
			std::clamp(v.y, minVal.y, maxVal.y)
		};
	}

	// Typedefs строго на движковых типах
	using Vec2f = Vec2<zF32>;
	using Vec2d = Vec2<zF64>;
	using Vec2i = Vec2<zI32>;
	using Vec2u = Vec2<zU32>;

	static_assert(std::is_standard_layout_v<Vec2<zF32>>, "Vec2 must be standard layout");
	static_assert(sizeof(Vec2<zF32>) == 8, "Vec2<zF32> must be 8 bytes");
}

template<zzz::math::Arithmetic T>
struct std::formatter<zzz::math::Vec2<T>> : std::formatter<std::string>
{
	auto format(const zzz::math::Vec2<T>& v, std::format_context& ctx) const
	{
		return std::formatter<std::string>::format(v.ToString(), ctx);
	}
};
