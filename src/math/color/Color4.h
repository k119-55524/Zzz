#pragma once

#include "math/color/Color3.h"

namespace zzz::math
{
	/**
	 * @class Color4
	 * @brief Шаблонный класс для хранения и управления 4-компонентным цветом (RGBA: Red, Green, Blue, Alpha).
	 *
	 * @tparam T Тип данных для хранения каналов цвета (должен быть арифметическим).
	 *           Значение по умолчанию: zF32.
	 */
	template<Arithmetic T = zF32>
	class Color4 final
	{
	public:
		T R; // Красный канал.
		T G; // Зеленый канал.
		T B; // Синий канал.
		T A; // Альфа канал (прозрачность).

		constexpr Color4() noexcept : R{ 0 }, G{ 0 }, B{ 0 }, A{ DefaultAlpha() } {}
		explicit constexpr Color4(T val) noexcept : R{ val }, G{ val }, B{ val }, A{ DefaultAlpha() } {}
		constexpr Color4(T r, T g, T b, T a = DefaultAlpha()) noexcept : R{ r }, G{ g }, B{ b }, A{ a } {}
		constexpr Color4(const Color3<T>& rgb, T a = DefaultAlpha()) noexcept : R{ rgb.R }, G{ rgb.G }, B{ rgb.B }, A{ a } {}
		constexpr Color4(const Color4& other) noexcept = default;
		constexpr Color4(Color4&&) noexcept = default;

		template<Arithmetic U>
		explicit constexpr Color4(U r, U g, U b, U a) noexcept
			: R{ details::ConvertChannel<T>(r) }
			, G{ details::ConvertChannel<T>(g) }
			, B{ details::ConvertChannel<T>(b) }
			, A{ details::ConvertChannel<T>(a) }
		{}

		template<Arithmetic U>
		explicit constexpr Color4(const Color4<U>& other) noexcept
			: R{ details::ConvertChannel<T>(other.R) }
			, G{ details::ConvertChannel<T>(other.G) }
			, B{ details::ConvertChannel<T>(other.B) }
			, A{ details::ConvertChannel<T>(other.A) }
		{}

		template<Arithmetic U>
		explicit constexpr Color4(const Color3<U>& rgb, T a = DefaultAlpha()) noexcept
			: R{ details::ConvertChannel<T>(rgb.R) }
			, G{ details::ConvertChannel<T>(rgb.G) }
			, B{ details::ConvertChannel<T>(rgb.B) }
			, A{ a }
		{}

		Color4& operator=(const Color4&) noexcept = default;
		Color4& operator=(Color4&&) noexcept = default;

		constexpr bool operator==(const Color4&) const noexcept = default;

		[[nodiscard]] constexpr T operator[](size_t index) const noexcept
		{
			assert(index < 4);
			return (&R)[index];
		}

		[[nodiscard]] constexpr T& operator[](size_t index) noexcept
		{
			assert(index < 4);
			return (&R)[index];
		}

		[[nodiscard]] constexpr operator const T* () const noexcept { return &R; }
		[[nodiscard]] constexpr operator T* () noexcept { return &R; }

		[[nodiscard]] constexpr Color3<T> GetRGB() const noexcept
		{
			return Color3<T>{ R, G, B };
		}

		[[nodiscard]] constexpr Color4 WithAlpha(T newA) const noexcept
		{
			return Color4{ R, G, B, newA };
		}

		constexpr Color4 operator+(const Color4& other) const noexcept
		{
			return Color4{ static_cast<T>(R + other.R), static_cast<T>(G + other.G), static_cast<T>(B + other.B), static_cast<T>(A + other.A) };
		}

		constexpr Color4 operator-(const Color4& other) const noexcept
		{
			return Color4{ static_cast<T>(R - other.R), static_cast<T>(G - other.G), static_cast<T>(B - other.B), static_cast<T>(A - other.A) };
		}

		constexpr Color4 operator*(const Color4& other) const noexcept
		{
			return Color4{ static_cast<T>(R * other.R), static_cast<T>(G * other.G), static_cast<T>(B * other.B), static_cast<T>(A * other.A) };
		}

		constexpr Color4 operator/(const Color4& other) const noexcept
		{
			return Color4{ static_cast<T>(R / other.R), static_cast<T>(G / other.G), static_cast<T>(B / other.B), static_cast<T>(A / other.A) };
		}

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Color4 operator*(S scalar) const noexcept
		{
			return Color4{ static_cast<T>(R * scalar), static_cast<T>(G * scalar), static_cast<T>(B * scalar), static_cast<T>(A * scalar) };
		}

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Color4 operator/(S scalar) const noexcept
		{
			return Color4{ static_cast<T>(R / scalar), static_cast<T>(G / scalar), static_cast<T>(B / scalar), static_cast<T>(A / scalar) };
		}

		constexpr Color4& operator+=(const Color4& other) noexcept { return *this = *this + other; }
		constexpr Color4& operator-=(const Color4& other) noexcept { return *this = *this - other; }
		constexpr Color4& operator*=(const Color4& other) noexcept { return *this = *this * other; }
		constexpr Color4& operator/=(const Color4& other) noexcept { return *this = *this / other; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Color4& operator*=(S scalar) noexcept { return *this = *this * scalar; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Color4& operator/=(S scalar) noexcept { return *this = *this / scalar; }

		inline void Clamp(T minVal = T(0), T maxVal = T(1)) noexcept
		{
			R = std::clamp(R, minVal, maxVal);
			G = std::clamp(G, minVal, maxVal);
			B = std::clamp(B, minVal, maxVal);
			A = std::clamp(A, minVal, maxVal);
		}

		[[nodiscard]] inline Color4 Clamped(T minVal = T(0), T maxVal = T(1)) const noexcept
		{
			Color4 res = *this;
			res.Clamp(minVal, maxVal);
			return res;
		}

		[[nodiscard]] inline Color4 Lerp(const Color4& target, zF32 t) const noexcept
		{
			const zF32 clampedT = std::clamp(t, 0.0f, 1.0f);
			if constexpr (std::is_floating_point_v<T>)
			{
				return Color4{
					static_cast<T>(R + (target.R - R) * clampedT),
					static_cast<T>(G + (target.G - G) * clampedT),
					static_cast<T>(B + (target.B - B) * clampedT),
					static_cast<T>(A + (target.A - A) * clampedT)
				};
			}
			else
			{
				const zF32 rF = static_cast<zF32>(R) + (static_cast<zF32>(target.R) - static_cast<zF32>(R)) * clampedT;
				const zF32 gF = static_cast<zF32>(G) + (static_cast<zF32>(target.G) - static_cast<zF32>(G)) * clampedT;
				const zF32 bF = static_cast<zF32>(B) + (static_cast<zF32>(target.B) - static_cast<zF32>(B)) * clampedT;
				const zF32 aF = static_cast<zF32>(A) + (static_cast<zF32>(target.A) - static_cast<zF32>(A)) * clampedT;
				return Color4{
					static_cast<T>(rF >= 0.0f ? rF + 0.5f : rF - 0.5f),
					static_cast<T>(gF >= 0.0f ? gF + 0.5f : gF - 0.5f),
					static_cast<T>(bF >= 0.0f ? bF + 0.5f : bF - 0.5f),
					static_cast<T>(aF >= 0.0f ? aF + 0.5f : aF - 0.5f)
				};
			}
		}

		inline void Set(T r, T g, T b, T a = DefaultAlpha()) noexcept
		{
			R = r;
			G = g;
			B = b;
			A = a;
		}

		template<Arithmetic U>
		inline void SetFrom(U r, U g, U b, U a) noexcept
		{
			R = details::ConvertChannel<T>(r);
			G = details::ConvertChannel<T>(g);
			B = details::ConvertChannel<T>(b);
			A = details::ConvertChannel<T>(a);
		}

		template<Arithmetic U>
		inline void SetFrom(const Color4<U>& other) noexcept
		{
			R = details::ConvertChannel<T>(other.R);
			G = details::ConvertChannel<T>(other.G);
			B = details::ConvertChannel<T>(other.B);
			A = details::ConvertChannel<T>(other.A);
		}

		[[nodiscard]] inline std::string ToString() const noexcept
		{
			return std::format("R: {}, G: {}, B: {}, A: {}", R, G, B, A);
		}

	private:
		[[nodiscard]] static constexpr T DefaultAlpha() noexcept
		{
			if constexpr (std::is_floating_point_v<T>) return T(1);
			else return T(255);
		}

		friend class zzz::core::Serializer;
	};

	template<Arithmetic T>
	constexpr Color3<T>::Color3(const Color4<T>& rgba) noexcept
		: R{ rgba.R }
		, G{ rgba.G }
		, B{ rgba.B }
	{}

	template<Arithmetic T>
	template<Arithmetic U>
	constexpr Color3<T>::Color3(const Color4<U>& rgba) noexcept
		: R{ details::ConvertChannel<T>(rgba.R) }
		, G{ details::ConvertChannel<T>(rgba.G) }
		, B{ details::ConvertChannel<T>(rgba.B) }
	{}

}
