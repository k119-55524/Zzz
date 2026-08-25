#pragma once

#include "math/MathIncludes.h"

namespace zzz::math
{
	template<Arithmetic T>
	class Color4;

	namespace details
	{
		template<typename To, typename From>
		constexpr To ConvertChannel(From val) noexcept
		{
			if constexpr (std::is_integral_v<From> && std::is_floating_point_v<To>)
			{
				return static_cast<To>(val) / To(255);
			}
			else if constexpr (std::is_floating_point_v<From> && std::is_integral_v<To>)
			{
				const From scaled = val * From(255);
				return static_cast<To>(scaled >= From(0) ? (scaled + From(0.5)) : (scaled - From(0.5)));
			}
			else
			{
				return static_cast<To>(val);
			}
		}
	}

	/**
	 * @class Color3
	 * @brief Шаблонный класс для хранения и управления 3-компонентным цветом (RGB: Red, Green, Blue).
	 *
	 * @tparam T Тип данных для хранения каналов цвета (должен быть арифметическим).
	 *           Значение по умолчанию: zF32.
	 */
	template<Arithmetic T = zF32>
	class Color3 final
	{
	public:
		T R; // Красный канал.
		T G; // Зеленый канал.
		T B; // Синий канал.

		constexpr Color3() noexcept : R{ 0 }, G{ 0 }, B{ 0 } {}
		explicit constexpr Color3(T val) noexcept : R{ val }, G{ val }, B{ val } {}
		constexpr Color3(T r, T g, T b) noexcept : R{ r }, G{ g }, B{ b } {}
		constexpr Color3(const Color3& other) noexcept = default;
		constexpr Color3(Color3&&) noexcept = default;

		template<Arithmetic U>
		explicit constexpr Color3(U r, U g, U b) noexcept
			: R{ details::ConvertChannel<T>(r) }
			, G{ details::ConvertChannel<T>(g) }
			, B{ details::ConvertChannel<T>(b) }
		{}

		constexpr Color3(const Color4<T>& rgba) noexcept;

		template<Arithmetic U>
		explicit constexpr Color3(const Color3<U>& other) noexcept
			: R{ details::ConvertChannel<T>(other.R) }
			, G{ details::ConvertChannel<T>(other.G) }
			, B{ details::ConvertChannel<T>(other.B) }
		{}

		template<Arithmetic U>
		explicit constexpr Color3(const Color4<U>& rgba) noexcept;

		Color3& operator=(const Color3&) noexcept = default;
		Color3& operator=(Color3&&) noexcept = default;

		constexpr bool operator==(const Color3&) const noexcept = default;

		[[nodiscard]] constexpr T operator[](size_t index) const noexcept
		{
			assert(index < 3);
			return (&R)[index];
		}

		[[nodiscard]] constexpr T& operator[](size_t index) noexcept
		{
			assert(index < 3);
			return (&R)[index];
		}

		[[nodiscard]] constexpr operator const T* () const noexcept { return &R; }
		[[nodiscard]] constexpr operator T* () noexcept { return &R; }

		constexpr Color3 operator+(const Color3& other) const noexcept
		{
			return Color3{ static_cast<T>(R + other.R), static_cast<T>(G + other.G), static_cast<T>(B + other.B) };
		}

		constexpr Color3 operator-(const Color3& other) const noexcept
		{
			return Color3{ static_cast<T>(R - other.R), static_cast<T>(G - other.G), static_cast<T>(B - other.B) };
		}

		constexpr Color3 operator*(const Color3& other) const noexcept
		{
			return Color3{ static_cast<T>(R * other.R), static_cast<T>(G * other.G), static_cast<T>(B * other.B) };
		}

		constexpr Color3 operator/(const Color3& other) const noexcept
		{
			return Color3{ static_cast<T>(R / other.R), static_cast<T>(G / other.G), static_cast<T>(B / other.B) };
		}

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Color3 operator*(S scalar) const noexcept
		{
			return Color3{ static_cast<T>(R * scalar), static_cast<T>(G * scalar), static_cast<T>(B * scalar) };
		}

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Color3 operator/(S scalar) const noexcept
		{
			return Color3{ static_cast<T>(R / scalar), static_cast<T>(G / scalar), static_cast<T>(B / scalar) };
		}

		constexpr Color3& operator+=(const Color3& other) noexcept { return *this = *this + other; }
		constexpr Color3& operator-=(const Color3& other) noexcept { return *this = *this - other; }
		constexpr Color3& operator*=(const Color3& other) noexcept { return *this = *this * other; }
		constexpr Color3& operator/=(const Color3& other) noexcept { return *this = *this / other; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Color3& operator*=(S scalar) noexcept { return *this = *this * scalar; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Color3& operator/=(S scalar) noexcept { return *this = *this / scalar; }

		inline void Clamp(T minVal = T(0), T maxVal = T(1)) noexcept
		{
			R = std::clamp(R, minVal, maxVal);
			G = std::clamp(G, minVal, maxVal);
			B = std::clamp(B, minVal, maxVal);
		}

		[[nodiscard]] inline Color3 Clamped(T minVal = T(0), T maxVal = T(1)) const noexcept
		{
			Color3 res = *this;
			res.Clamp(minVal, maxVal);
			return res;
		}

		[[nodiscard]] inline Color3 Lerp(const Color3& target, zF32 t) const noexcept
		{
			const zF32 clampedT = std::clamp(t, 0.0f, 1.0f);
			if constexpr (std::is_floating_point_v<T>)
			{
				return Color3{
					static_cast<T>(R + (target.R - R) * clampedT),
					static_cast<T>(G + (target.G - G) * clampedT),
					static_cast<T>(B + (target.B - B) * clampedT)
				};
			}
			else
			{
				const zF32 rF = static_cast<zF32>(R) + (static_cast<zF32>(target.R) - static_cast<zF32>(R)) * clampedT;
				const zF32 gF = static_cast<zF32>(G) + (static_cast<zF32>(target.G) - static_cast<zF32>(G)) * clampedT;
				const zF32 bF = static_cast<zF32>(B) + (static_cast<zF32>(target.B) - static_cast<zF32>(B)) * clampedT;
				return Color3{
					static_cast<T>(rF >= 0.0f ? rF + 0.5f : rF - 0.5f),
					static_cast<T>(gF >= 0.0f ? gF + 0.5f : gF - 0.5f),
					static_cast<T>(bF >= 0.0f ? bF + 0.5f : bF - 0.5f)
				};
			}
		}

		inline void Set(T r, T g, T b) noexcept
		{
			R = r;
			G = g;
			B = b;
		}

		template<Arithmetic U>
		inline void SetFrom(U r, U g, U b) noexcept
		{
			R = details::ConvertChannel<T>(r);
			G = details::ConvertChannel<T>(g);
			B = details::ConvertChannel<T>(b);
		}

		template<Arithmetic U>
		inline void SetFrom(const Color3<U>& other) noexcept
		{
			R = details::ConvertChannel<T>(other.R);
			G = details::ConvertChannel<T>(other.G);
			B = details::ConvertChannel<T>(other.B);
		}

		[[nodiscard]] inline std::string ToString() const noexcept
		{
			return std::format("R: {}, G: {}, B: {}", R, G, B);
		}

	private:
		friend class zzz::core::Serializer;
	};

}
