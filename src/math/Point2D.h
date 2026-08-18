#pragma once

#include "math/MathIncludes.h"
#include "math/Size2D.h"

namespace zzz::math
{
	/**
	 * @class Point2D
	 * @brief Шаблонный класс для хранения и управления двумерными координатами (X, Y).
	 *
	 * @tparam T Тип данных для хранения координат (должен быть арифметическим).
	 *           Значение по умолчанию: zI32.
	 */
	template<Arithmetic T>
	class Point2D final
	{
	public:
		constexpr Point2D() : m_X{ 0 }, m_Y{ 0 } {}
		explicit constexpr Point2D(T val) : m_X{ val }, m_Y{ val } {}
		constexpr Point2D(T x, T y) : m_X{ x }, m_Y{ y } {}
		constexpr Point2D(const Point2D& pt) : m_X{ pt.m_X }, m_Y{ pt.m_Y } {}
		constexpr Point2D(Point2D&&) noexcept = default;

		[[nodiscard]] inline T GetX() const noexcept { return m_X; }
		[[nodiscard]] inline T GetY() const noexcept { return m_Y; }

		inline void SetX(T x) noexcept { m_X = x; }
		inline void SetY(T y) noexcept { m_Y = y; }

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		inline void SetFrom(U x, U y) noexcept
		{
			m_X = static_cast<T>(x);
			m_Y = static_cast<T>(y);
		}

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		inline void SetFrom(const Point2D<U>& other) noexcept
		{
			m_X = static_cast<T>(other.GetX());
			m_Y = static_cast<T>(other.GetY());
		}

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		inline void SetFrom(const Size2D<U>& sz) noexcept
		{
			m_X = static_cast<T>(sz.GetWidth());
			m_Y = static_cast<T>(sz.GetHeight());
		}

		Point2D& operator=(const Point2D&) = default;
		Point2D& operator=(Point2D&&) noexcept = default;

		constexpr bool operator==(const Point2D&) const noexcept = default;

		constexpr Point2D operator+(const Point2D& other) const noexcept { return Point2D{ static_cast<T>(m_X + other.m_X), static_cast<T>(m_Y + other.m_Y) }; }
		constexpr Point2D operator-(const Point2D& other) const noexcept { return Point2D{ static_cast<T>(m_X - other.m_X), static_cast<T>(m_Y - other.m_Y) }; }

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		constexpr Point2D operator+(const Size2D<U>& sz) const noexcept { return Point2D{ static_cast<T>(m_X + sz.GetWidth()), static_cast<T>(m_Y + sz.GetHeight()) }; }

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		constexpr Point2D operator-(const Size2D<U>& sz) const noexcept { return Point2D{ static_cast<T>(m_X - sz.GetWidth()), static_cast<T>(m_Y - sz.GetHeight()) }; }

		constexpr Point2D& operator+=(const Point2D& other) noexcept { return *this = *this + other; }
		constexpr Point2D& operator-=(const Point2D& other) noexcept { return *this = *this - other; }

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		constexpr Point2D& operator+=(const Size2D<U>& sz) noexcept { return *this = *this + sz; }

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		constexpr Point2D& operator-=(const Size2D<U>& sz) noexcept { return *this = *this - sz; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Point2D operator*(S scale) const noexcept { return Point2D{ static_cast<T>(m_X * scale), static_cast<T>(m_Y * scale) }; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Point2D operator/(S scale) const noexcept { return Point2D{ static_cast<T>(m_X / scale), static_cast<T>(m_Y / scale) }; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Point2D& operator*=(S scale) noexcept { return *this = *this * scale; }

		template<Arithmetic S> requires std::is_arithmetic_v<S>
		constexpr Point2D& operator/=(S scale) noexcept { return *this = *this / scale; }

		[[nodiscard]] inline double DistanceTo(const Point2D& other) const noexcept
		{
			const double dx = static_cast<double>(m_X - other.m_X);
			const double dy = static_cast<double>(m_Y - other.m_Y);
			return std::sqrt(dx * dx + dy * dy);
		}

		inline void Offset(T dx, T dy) noexcept
		{
			m_X += dx;
			m_Y += dy;
		}

		[[nodiscard]] inline std::string ToString() const noexcept { return std::format("X: {}, Y: {}", m_X, m_Y); }

	private:
		T m_X; // Координата X.
		T m_Y; // Координата Y.

		friend class zzz::core::Serializer;
	};

	/// @brief Коммутативный оператор сложения: Size2D + Point2D -> Point2D
	template<Arithmetic T, Arithmetic U> requires SafelyConvertibleTo<U, T>
	constexpr Point2D<T> operator+(const Size2D<U>& sz, const Point2D<T>& pt) noexcept
	{
		return pt + sz;
	}
}
