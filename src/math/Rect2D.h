#pragma once

#include "math/MathIncludes.h"
#include "math/Point2D.h"
#include "math/Size2D.h"

namespace zzz::math
{
	/**
	 * @class Rect2D
	 * @brief Шаблонный класс для хранения и управления 2D прямоугольником (положение + размер).
	 *
	 * @tparam T Тип данных для координат и размеров (должен быть арифметическим).
	 *           Значение по умолчанию: zI32.
	 */
	template<Arithmetic T = zI32>
	class Rect2D final
	{
	public:
		using UnsignedT = std::make_unsigned_t<T>;

		constexpr Rect2D() : m_Position{}, m_Size{} {}
		constexpr Rect2D(T x, T y, T width, T height)
			: m_Position{ x, y }
			, m_Size{ static_cast<UnsignedT>(width), static_cast<UnsignedT>(height) }
		{}
		constexpr Rect2D(Point2D<T> pos, Size2D<UnsignedT> sz) : m_Position{ pos }, m_Size{ sz } {}
		constexpr Rect2D(Point2D<T> pos, Size2D<T> sz)
			: m_Position{ pos }
			, m_Size{ static_cast<UnsignedT>(sz.GetWidth()), static_cast<UnsignedT>(sz.GetHeight()) }
		{}
		constexpr Rect2D(const Rect2D&) = default;
		constexpr Rect2D(Rect2D&&) noexcept = default;

		[[nodiscard]] inline const Point2D<T>& GetPosition() const noexcept { return m_Position; }
		[[nodiscard]] inline const Size2D<UnsignedT>& GetSize() const noexcept { return m_Size; }

		inline void SetPosition(const Point2D<T>& pos) noexcept { m_Position = pos; }
		inline void SetSize(const Size2D<UnsignedT>& sz) noexcept { m_Size = sz; }

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		inline void SetFrom(const Rect2D<U>& other) noexcept
		{
			m_Position.SetFrom(other.GetPosition());
			m_Size.SetFrom(other.GetSize());
		}

		Rect2D& operator=(const Rect2D&) = default;
		Rect2D& operator=(Rect2D&&) noexcept = default;

		constexpr bool operator==(const Rect2D&) const noexcept = default;

		[[nodiscard]] inline T Left() const noexcept { return m_Position.GetX(); }
		[[nodiscard]] inline T Top() const noexcept { return m_Position.GetY(); }
		[[nodiscard]] inline T Right() const noexcept { return m_Position.GetX() + static_cast<T>(m_Size.GetWidth()); }
		[[nodiscard]] inline T Bottom() const noexcept { return m_Position.GetY() + static_cast<T>(m_Size.GetHeight()); }

		[[nodiscard]] inline std::string ToString() const noexcept
		{
			return std::format("X: {}, Y: {}, Width: {}, Height: {}", m_Position.GetX(), m_Position.GetY(), m_Size.GetWidth(), m_Size.GetHeight());
		}

	private:
		Point2D<T> m_Position;
		Size2D<UnsignedT> m_Size;

		friend class zzz::core::Serializer;
	};
}
