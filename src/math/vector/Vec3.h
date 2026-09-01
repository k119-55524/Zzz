#pragma once

#include "math/vector/Vec2.h"
#include "math/MathIncludes.h"

namespace zzz::math
{
	/**
	 * @struct Vec3
	 * @brief Шаблонная структура трехмерного вектора (X, Y, Z).
	 *        Система координат: Левосторонняя (Left-Handed: Y-up, Z-forward).
	 * @tparam T Арифметический тип данных (по умолчанию zF32).
	 */
	template<Arithmetic T = zF32>
	struct Vec3
	{
		T x{ static_cast<T>(0) };
		T y{ static_cast<T>(0) };
		T z{ static_cast<T>(0) };

		constexpr Vec3() noexcept = default;
		explicit constexpr Vec3(T scalar) noexcept : x{ scalar }, y{ scalar }, z{ scalar } {}
		constexpr Vec3(T inX, T inY, T inZ) noexcept : x{ inX }, y{ inY }, z{ inZ } {}
		constexpr Vec3(const Vec2<T>& xy, T inZ) noexcept : x{ xy.x }, y{ xy.y }, z{ inZ } {}

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		constexpr Vec3(const Vec3<U>& other) noexcept
			: x{ static_cast<T>(other.x) }
			, y{ static_cast<T>(other.y) }
			, z{ static_cast<T>(other.z) }
		{
		}

		[[nodiscard]] constexpr const T* data() const noexcept { return &x; }
		[[nodiscard]] constexpr T* data() noexcept { return &x; }

		[[nodiscard]] constexpr const T& operator[](size_t index) const noexcept
		{
			assert(index < 3 && "Vec3 index out of range");
			return (&x)[index];
		}

		[[nodiscard]] constexpr T& operator[](size_t index) noexcept
		{
			assert(index < 3 && "Vec3 index out of range");
			return (&x)[index];
		}

		[[nodiscard]] constexpr Vec3 operator+(const Vec3& other) const noexcept
		{
			return Vec3{ static_cast<T>(x + other.x), static_cast<T>(y + other.y), static_cast<T>(z + other.z) };
		}

		[[nodiscard]] constexpr Vec3 operator-(const Vec3& other) const noexcept
		{
			return Vec3{ static_cast<T>(x - other.x), static_cast<T>(y - other.y), static_cast<T>(z - other.z) };
		}

		[[nodiscard]] constexpr Vec3 operator*(const Vec3& other) const noexcept
		{
			return Vec3{ static_cast<T>(x * other.x), static_cast<T>(y * other.y), static_cast<T>(z * other.z) };
		}

		[[nodiscard]] constexpr Vec3 operator/(const Vec3& other) const noexcept
		{
			return Vec3{ static_cast<T>(x / other.x), static_cast<T>(y / other.y), static_cast<T>(z / other.z) };
		}

		template<Arithmetic S>
		[[nodiscard]] constexpr Vec3 operator*(S scalar) const noexcept
		{
			return Vec3{ static_cast<T>(x * scalar), static_cast<T>(y * scalar), static_cast<T>(z * scalar) };
		}

		template<Arithmetic S>
		[[nodiscard]] constexpr Vec3 operator/(S scalar) const noexcept
		{
			return Vec3{ static_cast<T>(x / scalar), static_cast<T>(y / scalar), static_cast<T>(z / scalar) };
		}

		[[nodiscard]] constexpr Vec3 operator-() const noexcept
		{
			return Vec3{ static_cast<T>(-x), static_cast<T>(-y), static_cast<T>(-z) };
		}

		constexpr Vec3& operator+=(const Vec3& other) noexcept
		{
			x = static_cast<T>(x + other.x);
			y = static_cast<T>(y + other.y);
			z = static_cast<T>(z + other.z);
			return *this;
		}

		constexpr Vec3& operator-=(const Vec3& other) noexcept
		{
			x = static_cast<T>(x - other.x);
			y = static_cast<T>(y - other.y);
			z = static_cast<T>(z - other.z);
			return *this;
		}

		constexpr Vec3& operator*=(const Vec3& other) noexcept
		{
			x = static_cast<T>(x * other.x);
			y = static_cast<T>(y * other.y);
			z = static_cast<T>(z * other.z);
			return *this;
		}

		constexpr Vec3& operator/=(const Vec3& other) noexcept
		{
			x = static_cast<T>(x / other.x);
			y = static_cast<T>(y / other.y);
			z = static_cast<T>(z / other.z);
			return *this;
		}

		template<Arithmetic S>
		constexpr Vec3& operator*=(S scalar) noexcept
		{
			x = static_cast<T>(x * scalar);
			y = static_cast<T>(y * scalar);
			z = static_cast<T>(z * scalar);
			return *this;
		}

		template<Arithmetic S>
		constexpr Vec3& operator/=(S scalar) noexcept
		{
			x = static_cast<T>(x / scalar);
			y = static_cast<T>(y / scalar);
			z = static_cast<T>(z / scalar);
			return *this;
		}

		[[nodiscard]] constexpr bool operator==(const Vec3& other) const noexcept
		{
			if constexpr (std::floating_point<T>)
			{
				constexpr T eps = static_cast<T>(1e-5);
				return std::abs(x - other.x) <= eps &&
					   std::abs(y - other.y) <= eps &&
					   std::abs(z - other.z) <= eps;
			}
			else
			{
				return x == other.x && y == other.y && z == other.z;
			}
		}

		[[nodiscard]] constexpr bool operator!=(const Vec3& other) const noexcept
		{
			return !(*this == other);
		}

		[[nodiscard]] constexpr T Dot(const Vec3& other) const noexcept
		{
			return static_cast<T>(x * other.x + y * other.y + z * other.z);
		}

		[[nodiscard]] constexpr Vec3 Cross(const Vec3& other) const noexcept
		{
			return Vec3{
				static_cast<T>(y * other.z - z * other.y),
				static_cast<T>(z * other.x - x * other.z),
				static_cast<T>(x * other.y - y * other.x)
			};
		}

		[[nodiscard]] constexpr T LengthSquared() const noexcept
		{
			return Dot(*this);
		}

		[[nodiscard]] auto Length() const noexcept
		{
			return std::sqrt(LengthSquared());
		}

		Vec3& Normalize() noexcept requires std::floating_point<T>
		{
			const T len = Length();
			if (len > static_cast<T>(1e-6))
			{
				x /= len;
				y /= len;
				z /= len;
			}
			else
			{
				x = static_cast<T>(0);
				y = static_cast<T>(0);
				z = static_cast<T>(0);
			}
			return *this;
		}

		[[nodiscard]] Vec3 Normalized() const noexcept requires std::floating_point<T>
		{
			Vec3 result = *this;
			return result.Normalize();
		}

		[[nodiscard]] auto Distance(const Vec3& other) const noexcept
		{
			return (*this - other).Length();
		}

		[[nodiscard]] constexpr Vec3 Lerp(const Vec3& target, zF32 t) const noexcept
		{
			return Vec3{
				static_cast<T>(x + (target.x - x) * t),
				static_cast<T>(y + (target.y - y) * t),
				static_cast<T>(z + (target.z - z) * t)
			};
		}

		[[nodiscard]] Vec3 Reflect(const Vec3& normal) const noexcept requires std::floating_point<T>
		{
			return *this - normal * (static_cast<T>(2) * Dot(normal));
		}

		[[nodiscard]] std::string ToString() const
		{
			return std::format("Vec3({}, {}, {})", x, y, z);
		}

		// Левосторонняя система координат (Left-Handed: Y-up, Z-forward, X-right)
		[[nodiscard]] static constexpr Vec3 Zero() noexcept { return Vec3{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) }; }
		[[nodiscard]] static constexpr Vec3 One() noexcept { return Vec3{ static_cast<T>(1), static_cast<T>(1), static_cast<T>(1) }; }
		[[nodiscard]] static constexpr Vec3 Up() noexcept { return Vec3{ static_cast<T>(0), static_cast<T>(1), static_cast<T>(0) }; }
		[[nodiscard]] static constexpr Vec3 Down() noexcept { return Vec3{ static_cast<T>(0), static_cast<T>(-1), static_cast<T>(0) }; }
		[[nodiscard]] static constexpr Vec3 Left() noexcept { return Vec3{ static_cast<T>(-1), static_cast<T>(0), static_cast<T>(0) }; }
		[[nodiscard]] static constexpr Vec3 Right() noexcept { return Vec3{ static_cast<T>(1), static_cast<T>(0), static_cast<T>(0) }; }
		[[nodiscard]] static constexpr Vec3 Forward() noexcept { return Vec3{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(1) }; }
		[[nodiscard]] static constexpr Vec3 Back() noexcept { return Vec3{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(-1) }; }
	};

	template<Arithmetic S, Arithmetic T>
	[[nodiscard]] constexpr Vec3<T> operator*(S scalar, const Vec3<T>& vec) noexcept
	{
		return vec * scalar;
	}

	template<Arithmetic T>
	[[nodiscard]] constexpr T Dot(const Vec3<T>& a, const Vec3<T>& b) noexcept
	{
		return a.Dot(b);
	}

	template<Arithmetic T>
	[[nodiscard]] constexpr Vec3<T> Cross(const Vec3<T>& a, const Vec3<T>& b) noexcept
	{
		return a.Cross(b);
	}

	template<Arithmetic T>
	[[nodiscard]] auto Distance(const Vec3<T>& a, const Vec3<T>& b) noexcept
	{
		return a.Distance(b);
	}

	template<Arithmetic T>
	[[nodiscard]] constexpr Vec3<T> Lerp(const Vec3<T>& a, const Vec3<T>& b, zF32 t) noexcept
	{
		return a.Lerp(b, t);
	}

	template<Arithmetic T>
	[[nodiscard]] constexpr Vec3<T> Min(const Vec3<T>& a, const Vec3<T>& b) noexcept
	{
		return Vec3<T>{ std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) };
	}

	template<Arithmetic T>
	[[nodiscard]] constexpr Vec3<T> Max(const Vec3<T>& a, const Vec3<T>& b) noexcept
	{
		return Vec3<T>{ std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) };
	}

	template<Arithmetic T>
	[[nodiscard]] constexpr Vec3<T> Clamp(const Vec3<T>& v, const Vec3<T>& minVal, const Vec3<T>& maxVal) noexcept
	{
		return Vec3<T>{
			std::clamp(v.x, minVal.x, maxVal.x),
			std::clamp(v.y, minVal.y, maxVal.y),
			std::clamp(v.z, minVal.z, maxVal.z)
		};
	}

	// Typedefs строго на движковых типах
	using Vec3f = Vec3<zF32>;
	using Vec3d = Vec3<zF64>;
	using Vec3i = Vec3<zI32>;
	using Vec3u = Vec3<zU32>;

	static_assert(std::is_standard_layout_v<Vec3<zF32>>, "Vec3 must be standard layout");
	static_assert(sizeof(Vec3<zF32>) == 12, "Vec3<zF32> must be 12 bytes");
}

template<zzz::math::Arithmetic T>
struct std::formatter<zzz::math::Vec3<T>> : std::formatter<std::string>
{
	auto format(const zzz::math::Vec3<T>& v, std::format_context& ctx) const
	{
		return std::formatter<std::string>::format(v.ToString(), ctx);
	}
};
