#pragma once

#include "math/MathIncludes.h"
#include "math/vector/Vec2.h"
#include "math/vector/Vec3.h"

namespace zzz::math
{
	/**
	 * @struct Vec4
	 * @brief Шаблонная структура четырехмерного вектора (X, Y, Z, W).
	 *        Используется для однородных координат, кватернионов, RGBA цветов и тангент-векторов.
	 * @tparam T Арифметический тип данных (по умолчанию zF32).
	 */
	template<Arithmetic T = zF32>
	struct Vec4
	{
		T x{ static_cast<T>(0) };
		T y{ static_cast<T>(0) };
		T z{ static_cast<T>(0) };
		T w{ static_cast<T>(0) };

		constexpr Vec4() noexcept = default;
		explicit constexpr Vec4(T scalar) noexcept : x{ scalar }, y{ scalar }, z{ scalar }, w{ scalar } {}
		constexpr Vec4(T inX, T inY, T inZ, T inW) noexcept : x{ inX }, y{ inY }, z{ inZ }, w{ inW } {}
		constexpr Vec4(const Vec2<T>& xy, T inZ, T inW) noexcept : x{ xy.x }, y{ xy.y }, z{ inZ }, w{ inW } {}
		constexpr Vec4(const Vec3<T>& xyz, T inW) noexcept : x{ xyz.x }, y{ xyz.y }, z{ xyz.z }, w{ inW } {}

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		constexpr Vec4(const Vec4<U>& other) noexcept
			: x{ static_cast<T>(other.x) }
			, y{ static_cast<T>(other.y) }
			, z{ static_cast<T>(other.z) }
			, w{ static_cast<T>(other.w) }
		{
		}

		[[nodiscard]] constexpr const T* data() const noexcept { return &x; }
		[[nodiscard]] constexpr T* data() noexcept { return &x; }

		[[nodiscard]] constexpr const T& operator[](size_t index) const noexcept
		{
			assert(index < 4 && "Vec4 index out of range");
			return (&x)[index];
		}

		[[nodiscard]] constexpr T& operator[](size_t index) noexcept
		{
			assert(index < 4 && "Vec4 index out of range");
			return (&x)[index];
		}

		[[nodiscard]] constexpr Vec4 operator+(const Vec4& other) const noexcept
		{
			return Vec4{ static_cast<T>(x + other.x), static_cast<T>(y + other.y), static_cast<T>(z + other.z), static_cast<T>(w + other.w) };
		}

		[[nodiscard]] constexpr Vec4 operator-(const Vec4& other) const noexcept
		{
			return Vec4{ static_cast<T>(x - other.x), static_cast<T>(y - other.y), static_cast<T>(z - other.z), static_cast<T>(w - other.w) };
		}

		[[nodiscard]] constexpr Vec4 operator*(const Vec4& other) const noexcept
		{
			return Vec4{ static_cast<T>(x * other.x), static_cast<T>(y * other.y), static_cast<T>(z * other.z), static_cast<T>(w * other.w) };
		}

		[[nodiscard]] constexpr Vec4 operator/(const Vec4& other) const noexcept
		{
			return Vec4{ static_cast<T>(x / other.x), static_cast<T>(y / other.y), static_cast<T>(z / other.z), static_cast<T>(w / other.w) };
		}

		template<Arithmetic S>
		[[nodiscard]] constexpr Vec4 operator*(S scalar) const noexcept
		{
			return Vec4{ static_cast<T>(x * scalar), static_cast<T>(y * scalar), static_cast<T>(z * scalar), static_cast<T>(w * scalar) };
		}

		template<Arithmetic S>
		[[nodiscard]] constexpr Vec4 operator/(S scalar) const noexcept
		{
			return Vec4{ static_cast<T>(x / scalar), static_cast<T>(y / scalar), static_cast<T>(z / scalar), static_cast<T>(w / scalar) };
		}

		[[nodiscard]] constexpr Vec4 operator-() const noexcept
		{
			return Vec4{ static_cast<T>(-x), static_cast<T>(-y), static_cast<T>(-z), static_cast<T>(-w) };
		}

		constexpr Vec4& operator+=(const Vec4& other) noexcept
		{
			x = static_cast<T>(x + other.x);
			y = static_cast<T>(y + other.y);
			z = static_cast<T>(z + other.z);
			w = static_cast<T>(w + other.w);
			return *this;
		}

		constexpr Vec4& operator-=(const Vec4& other) noexcept
		{
			x = static_cast<T>(x - other.x);
			y = static_cast<T>(y - other.y);
			z = static_cast<T>(z - other.z);
			w = static_cast<T>(w - other.w);
			return *this;
		}

		constexpr Vec4& operator*=(const Vec4& other) noexcept
		{
			x = static_cast<T>(x * other.x);
			y = static_cast<T>(y * other.y);
			z = static_cast<T>(z * other.z);
			w = static_cast<T>(w * other.w);
			return *this;
		}

		constexpr Vec4& operator/=(const Vec4& other) noexcept
		{
			x = static_cast<T>(x / other.x);
			y = static_cast<T>(y / other.y);
			z = static_cast<T>(z / other.z);
			w = static_cast<T>(w / other.w);
			return *this;
		}

		template<Arithmetic S>
		constexpr Vec4& operator*=(S scalar) noexcept
		{
			x = static_cast<T>(x * scalar);
			y = static_cast<T>(y * scalar);
			z = static_cast<T>(z * scalar);
			w = static_cast<T>(w * scalar);
			return *this;
		}

		template<Arithmetic S>
		constexpr Vec4& operator/=(S scalar) noexcept
		{
			x = static_cast<T>(x / scalar);
			y = static_cast<T>(y / scalar);
			z = static_cast<T>(z / scalar);
			w = static_cast<T>(w / scalar);
			return *this;
		}

		[[nodiscard]] constexpr bool operator==(const Vec4& other) const noexcept
		{
			if constexpr (std::floating_point<T>)
			{
				constexpr T eps = static_cast<T>(1e-5);
				return std::abs(x - other.x) <= eps &&
					   std::abs(y - other.y) <= eps &&
					   std::abs(z - other.z) <= eps &&
					   std::abs(w - other.w) <= eps;
			}
			else
			{
				return x == other.x && y == other.y && z == other.z && w == other.w;
			}
		}

		[[nodiscard]] constexpr bool operator!=(const Vec4& other) const noexcept
		{
			return !(*this == other);
		}

		[[nodiscard]] constexpr T Dot(const Vec4& other) const noexcept
		{
			return static_cast<T>(x * other.x + y * other.y + z * other.z + w * other.w);
		}

		[[nodiscard]] constexpr T LengthSquared() const noexcept
		{
			return Dot(*this);
		}

		[[nodiscard]] auto Length() const noexcept
		{
			return std::sqrt(LengthSquared());
		}

		Vec4& Normalize() noexcept requires std::floating_point<T>
		{
			const T len = Length();
			if (len > static_cast<T>(1e-6))
			{
				x /= len;
				y /= len;
				z /= len;
				w /= len;
			}
			else
			{
				x = static_cast<T>(0);
				y = static_cast<T>(0);
				z = static_cast<T>(0);
				w = static_cast<T>(0);
			}
			return *this;
		}

		[[nodiscard]] Vec4 Normalized() const noexcept requires std::floating_point<T>
		{
			Vec4 result = *this;
			return result.Normalize();
		}

		[[nodiscard]] auto Distance(const Vec4& other) const noexcept
		{
			return (*this - other).Length();
		}

		[[nodiscard]] constexpr Vec4 Lerp(const Vec4& target, zF32 t) const noexcept
		{
			return Vec4{
				static_cast<T>(x + (target.x - x) * t),
				static_cast<T>(y + (target.y - y) * t),
				static_cast<T>(z + (target.z - z) * t),
				static_cast<T>(w + (target.w - w) * t)
			};
		}

		[[nodiscard]] std::string ToString() const
		{
			return std::format("Vec4({}, {}, {}, {})", x, y, z, w);
		}

		[[nodiscard]] static constexpr Vec4 Zero() noexcept { return Vec4{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) }; }
		[[nodiscard]] static constexpr Vec4 One() noexcept { return Vec4{ static_cast<T>(1), static_cast<T>(1), static_cast<T>(1), static_cast<T>(1) }; }
		[[nodiscard]] static constexpr Vec4 UnitX() noexcept { return Vec4{ static_cast<T>(1), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) }; }
		[[nodiscard]] static constexpr Vec4 UnitY() noexcept { return Vec4{ static_cast<T>(0), static_cast<T>(1), static_cast<T>(0), static_cast<T>(0) }; }
		[[nodiscard]] static constexpr Vec4 UnitZ() noexcept { return Vec4{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(1), static_cast<T>(0) }; }
		[[nodiscard]] static constexpr Vec4 UnitW() noexcept { return Vec4{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1) }; }
	};

	template<Arithmetic S, Arithmetic T>
	[[nodiscard]] constexpr Vec4<T> operator*(S scalar, const Vec4<T>& vec) noexcept
	{
		return vec * scalar;
	}

	template<Arithmetic T>
	[[nodiscard]] constexpr T Dot(const Vec4<T>& a, const Vec4<T>& b) noexcept
	{
		return a.Dot(b);
	}

	template<Arithmetic T>
	[[nodiscard]] auto Distance(const Vec4<T>& a, const Vec4<T>& b) noexcept
	{
		return a.Distance(b);
	}

	template<Arithmetic T>
	[[nodiscard]] constexpr Vec4<T> Lerp(const Vec4<T>& a, const Vec4<T>& b, zF32 t) noexcept
	{
		return a.Lerp(b, t);
	}

	template<Arithmetic T>
	[[nodiscard]] constexpr Vec4<T> Min(const Vec4<T>& a, const Vec4<T>& b) noexcept
	{
		return Vec4<T>{ std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w) };
	}

	template<Arithmetic T>
	[[nodiscard]] constexpr Vec4<T> Max(const Vec4<T>& a, const Vec4<T>& b) noexcept
	{
		return Vec4<T>{ std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w) };
	}

	template<Arithmetic T>
	[[nodiscard]] constexpr Vec4<T> Clamp(const Vec4<T>& v, const Vec4<T>& minVal, const Vec4<T>& maxVal) noexcept
	{
		return Vec4<T>{
			std::clamp(v.x, minVal.x, maxVal.x),
			std::clamp(v.y, minVal.y, maxVal.y),
			std::clamp(v.z, minVal.z, maxVal.z),
			std::clamp(v.w, minVal.w, maxVal.w)
		};
	}

	// Typedefs строго на движковых типах
	using Vec4f = Vec4<zF32>;
	using Vec4d = Vec4<zF64>;
	using Vec4i = Vec4<zI32>;
	using Vec4u = Vec4<zU32>;

	static_assert(std::is_standard_layout_v<Vec4<zF32>>, "Vec4 must be standard layout");
	static_assert(sizeof(Vec4<zF32>) == 16, "Vec4<zF32> must be 16 bytes");
}

template<zzz::math::Arithmetic T>
struct std::formatter<zzz::math::Vec4<T>> : std::formatter<std::string>
{
	auto format(const zzz::math::Vec4<T>& v, std::format_context& ctx) const
	{
		return std::formatter<std::string>::format(v.ToString(), ctx);
	}
};
