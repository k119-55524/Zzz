#pragma once

#include "math/MathIncludes.h"

namespace zzz::math
{
	template<Arithmetic T = zI32>
	class Point2D;

	/**
	 * @class Size2D
	 * @brief Шаблонный класс для хранения и управления двумерными размерами.
	 *
	 * @tparam T Тип данных для хранения ширины и высоты (должен быть арифметическим).
	 *           Значение по умолчанию: zU32.
	 */
	template<Arithmetic T = zU32>
	class Size2D final
	{
	public:
		constexpr Size2D() : m_Width{ 0 }, m_Height{ 0 } {}
		explicit constexpr Size2D(T size) : m_Width{ size }, m_Height{ size } {}
		constexpr Size2D(T width, T height) : m_Width{ width }, m_Height{ height } {}
		constexpr Size2D(const Size2D& size) : m_Width{ size.m_Width }, m_Height{ size.m_Height } {}
		constexpr Size2D(Size2D&&) noexcept = default;

		[[nodiscard]] inline T GetWidth() const noexcept { return m_Width; }
		[[nodiscard]] inline T GetHeight() const noexcept { return m_Height; }

		inline void SetWidth(T width) noexcept { m_Width = width; }
		inline void SetHeight(T height) noexcept { m_Height = height; }

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		inline void SetFrom(U w, U h) noexcept
		{
			m_Width = static_cast<T>(w);
			m_Height = static_cast<T>(h);
		}

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		inline void SetFrom(const Size2D<U>& other) noexcept
		{
			m_Width = static_cast<T>(other.GetWidth());
			m_Height = static_cast<T>(other.GetHeight());
		}

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		inline void SetFrom(const Point2D<U>& pt) noexcept;

		Size2D& operator=(const Size2D&) = default;
		Size2D& operator=(Size2D&&) noexcept = default;

		constexpr bool operator==(const Size2D&) const noexcept = default;

		constexpr Size2D operator+(const Size2D& other) const noexcept { return Size2D{ static_cast<T>(m_Width + other.m_Width), static_cast<T>(m_Height + other.m_Height) }; }
		constexpr Size2D operator-(const Size2D& other) const noexcept { return Size2D{ static_cast<T>(m_Width - other.m_Width), static_cast<T>(m_Height - other.m_Height) }; }

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		constexpr Size2D operator+(const Size2D<U>& other) const noexcept { return Size2D{ static_cast<T>(m_Width + other.GetWidth()), static_cast<T>(m_Height + other.GetHeight()) }; }

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		constexpr Size2D operator-(const Size2D<U>& other) const noexcept { return Size2D{ static_cast<T>(m_Width - other.GetWidth()), static_cast<T>(m_Height - other.GetHeight()) }; }

		constexpr Size2D& operator+=(const Size2D& other) noexcept { return *this = *this + other; }
		constexpr Size2D& operator-=(const Size2D& other) noexcept { return *this = *this - other; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Size2D operator*(S scale) const noexcept { return Size2D{ static_cast<T>(m_Width * scale), static_cast<T>(m_Height * scale) }; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Size2D operator/(S scale) const noexcept { return Size2D{ static_cast<T>(m_Width / scale), static_cast<T>(m_Height / scale) }; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Size2D& operator*=(S scale) noexcept { return *this = *this * scale; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Size2D& operator/=(S scale) noexcept { return *this = *this / scale; }

		[[nodiscard]] inline std::string ToString() const noexcept { return std::format("Width: {}, Height: {}", m_Width, m_Height); }

	private:
		T m_Width;  // Ширина объекта.
		T m_Height; // Высота объекта.

		friend class zzz::core::Serializer;
	};
}

#include "math/Point2D.h"

namespace zzz::math
{
	template<Arithmetic T>
	template<Arithmetic U> requires SafelyConvertibleTo<U, T>
	inline void Size2D<T>::SetFrom(const Point2D<U>& pt) noexcept
	{
		m_Width = static_cast<T>(pt.GetX());
		m_Height = static_cast<T>(pt.GetY());
	}
}
