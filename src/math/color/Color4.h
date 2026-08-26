#pragma once

#include "math/MathIncludes.h"
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
		T R; ///< Красный канал (Red).
		T G; ///< Зеленый канал (Green).
		T B; ///< Синий канал (Blue).
		T A; ///< Альфа канал (Alpha / Прозрачность).

		/// @brief Конструктор по умолчанию. Инициализирует R, G, B нулями (чёрный цвет) и A максимальной непрозрачностью (1.0f или 255).
		constexpr Color4() noexcept : R{ 0 }, G{ 0 }, B{ 0 }, A{ DefaultAlpha() } {}

		/// @brief Конструктор с инициализацией цветовых каналов одинаковым значением.
		/// @param val Значение для R, G, B. Альфа инициализируется максимальной непрозрачностью.
		explicit constexpr Color4(T val) noexcept : R{ val }, G{ val }, B{ val }, A{ DefaultAlpha() } {}

		/// @brief Конструктор с явным указанием каждого компонента цвета.
		/// @param r Значение красного канала.
		/// @param g Значение зеленого канала.
		/// @param b Значение синего канала.
		/// @param a Значение альфа-канала (по умолчанию 1.0f или 255).
		constexpr Color4(T r, T g, T b, T a = DefaultAlpha()) noexcept : R{ r }, G{ g }, B{ b }, A{ a } {}

		/// @brief Конструктор создания 4-компонентного цвета из 3-компонентного Color3 с указанием альфа-канала.
		/// @param rgb Исходный цвет Color3 (RGB).
		/// @param a Альфа-канал прозрачности.
		constexpr Color4(const Color3<T>& rgb, T a = DefaultAlpha()) noexcept : R{ rgb.R }, G{ rgb.G }, B{ rgb.B }, A{ a } {}

		/// @brief Конструктор копирования по умолчанию.
		constexpr Color4(const Color4& other) noexcept = default;

		/// @brief Конструктор перемещения по умолчанию.
		constexpr Color4(Color4&&) noexcept = default;

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

		/// @brief Возвращает RGB-компоненты цвета в виде объекта Color3<T> (отбрасывая альфа-канал).
		/// @return Цвет Color3<T>.
		[[nodiscard]] constexpr Color3<T> GetRGB() const noexcept
		{
			return Color3<T>{ R, G, B };
		}

		/// @brief Возвращает новый объект Color4 с изменённым значением альфа-канала.
		/// @param newA Новое значение прозрачности.
		/// @return Скопированный цвет с обновленной альфой.
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

		/// @brief Ограничивает все 4 канала цвета в текущем объекте диапазоном [minVal, maxVal].
		/// @param minVal Минимальная граница (по умолчанию 0).
		/// @param maxVal Максимальная граница (по умолчанию 1).
		inline void Clamp(T minVal = T(0), T maxVal = T(1)) noexcept
		{
			R = std::clamp(R, minVal, maxVal);
			G = std::clamp(G, minVal, maxVal);
			B = std::clamp(B, minVal, maxVal);
			A = std::clamp(A, minVal, maxVal);
		}

		/// @brief Возвращает новый цвет с ограниченными диапазоном [minVal, maxVal] каналами.
		/// @param minVal Минимальная граница (по умолчанию 0).
		/// @param maxVal Максимальная граница (по умолчанию 1).
		/// @return Скопированный и ограниченный цвет.
		[[nodiscard]] inline Color4 Clamped(T minVal = T(0), T maxVal = T(1)) const noexcept
		{
			Color4 res = *this;
			res.Clamp(minVal, maxVal);
			return res;
		}

		/// @brief Выполняет линейную интерполяцию (Lerp) по всем 4 каналам между текущим и целевым цветом.
		/// @param target Целевой цвет.
		/// @param t Коэффициент интерполяции (автоматически зажимается в диапазон [0.0f, 1.0f]).
		/// @return Интерполированный цвет.
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

		/// @brief Устанавливает новые значения каналов текущего цвета без конвертации типов.
		/// @param r Красный канал.
		/// @param g Зеленый канал.
		/// @param b Синий канал.
		/// @param a Альфа-канал.
		inline void Set(T r, T g, T b, T a = DefaultAlpha()) noexcept
		{
			R = r;
			G = g;
			B = b;
			A = a;
		}

		/// @brief Задаёт новые значения каналов из другого арифметического типа U с автоматической масштабируемой конвертацией диапазонов.
		/// @tparam U Исходный арифметический тип компонентов.
		/// @param r Красный канал типа U.
		/// @param g Зеленый канал типа U.
		/// @param b Синий канал типа U.
		/// @param a Альфа-канал типа U.
		template<Arithmetic U>
		inline void SetFrom(U r, U g, U b, U a = DefaultAlphaU<U>()) noexcept
		{
			R = ConvertChannel<T>(r);
			G = ConvertChannel<T>(g);
			B = ConvertChannel<T>(b);
			A = ConvertChannel<T>(a);
		}

		/// @brief Задаёт значения из объекта Color4 другого типа U с автоматической масштабируемой конвертацией диапазонов.
		/// @tparam U Исходный арифметический тип исходного цвета.
		/// @param other Исходный цвет типа U.
		template<Arithmetic U>
		inline void SetFrom(const Color4<U>& other) noexcept
		{
			R = ConvertChannel<T>(other.R);
			G = ConvertChannel<T>(other.G);
			B = ConvertChannel<T>(other.B);
			A = ConvertChannel<T>(other.A);
		}

		/// @brief Преобразует и возвращает новый цвет типа TargetT с автоматической масштабируемой конвертацией каналов.
		/// @tparam TargetT Целевой арифметический тип каналов цветности.
		/// @return Новый объект Color4<TargetT>.
		template<Arithmetic TargetT>
		[[nodiscard]] inline Color4<TargetT> ConvertTo() const noexcept
		{
			return Color4<TargetT>{
				ConvertChannel<TargetT>(R),
				ConvertChannel<TargetT>(G),
				ConvertChannel<TargetT>(B),
				ConvertChannel<TargetT>(A)
			};
		}

		/// @brief Формирует строковое представление цвета.
		/// @return Строка формата "R: ..., G: ..., B: ..., A: ...".
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

		template<Arithmetic U>
		[[nodiscard]] static constexpr U DefaultAlphaU() noexcept
		{
			if constexpr (std::is_floating_point_v<U>) return U(1);
			else return U(255);
		}

		friend class zzz::core::Serializer;
	};
}
