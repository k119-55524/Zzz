#pragma once

#include "math/MathIncludes.h"
#include "math/Point2D.h"
#include "math/Size2D.h"

namespace zzz::math
{
	/**
	 * @class Rect2D
	 * @brief Шаблонный класс для хранения и управления 2D прямоугольником (position + size).
	 *
	 * @tparam T Тип данных для координат и размеров (должен быть арифметическим).
	 *           Значение по умолчанию: zI32.
	 */
	template<Arithmetic T = zI32>
	class Rect2D final
	{
	public:
		using UnsignedT = std::make_unsigned_t<T>;

		Point2D<T> position{};
		Size2D<UnsignedT> size{};

		constexpr Rect2D() noexcept = default;
		constexpr Rect2D(T x, T y, T width, T height) noexcept
			: position{ x, y }
			, size{ static_cast<UnsignedT>(width), static_cast<UnsignedT>(height) }
		{}
		constexpr Rect2D(Point2D<T> pos, Size2D<UnsignedT> sz) noexcept : position{ pos }, size{ sz } {}
		constexpr Rect2D(Point2D<T> pos, Size2D<T> sz) noexcept
			: position{ pos }
			, size{ static_cast<UnsignedT>(sz.width), static_cast<UnsignedT>(sz.height) }
		{}
		constexpr Rect2D(const Rect2D&) = default;
		constexpr Rect2D(Rect2D&&) noexcept = default;

		[[nodiscard]] inline const Point2D<T>& GetPosition() const noexcept { return position; }
		[[nodiscard]] inline const Size2D<UnsignedT>& GetSize() const noexcept { return size; }

		inline void SetPosition(const Point2D<T>& pos) noexcept { position = pos; }
		inline void SetSize(const Size2D<UnsignedT>& sz) noexcept { size = sz; }

		template<Arithmetic U> requires SafelyConvertibleTo<U, T>
		inline void SetFrom(const Rect2D<U>& other) noexcept
		{
			position.SetFrom(other.GetPosition());
			size.SetFrom(other.GetSize());
		}

		Rect2D& operator=(const Rect2D&) = default;
		Rect2D& operator=(Rect2D&&) noexcept = default;

		constexpr bool operator==(const Rect2D&) const noexcept = default;

		[[nodiscard]] inline T Left() const noexcept { return position.x; }
		[[nodiscard]] inline T Top() const noexcept { return position.y; }
		[[nodiscard]] inline T Right() const noexcept { return position.x + static_cast<T>(size.width); }
		[[nodiscard]] inline T Bottom() const noexcept { return position.y + static_cast<T>(size.height); }

		template<Arithmetic U1, Arithmetic U2, Arithmetic U3, Arithmetic U4>
		inline void SetFrom(U1 inX, U2 inY, U3 inW, U4 inH) noexcept
		{
			position.SetFrom(inX, inY);
			size.SetFrom(inW, inH);
		}

		[[nodiscard]] inline bool Contains(T inX, T inY) const noexcept
		{
			return inX >= Left() && inX < Right() && inY >= Top() && inY < Bottom();
		}

		[[nodiscard]] inline bool Intersects(const Rect2D& other) const noexcept
		{
			return Left() < other.Right() && Right() > other.Left() &&
				Top() < other.Bottom() && Bottom() > other.Top();
		}

		[[nodiscard]] inline std::string ToString() const noexcept
		{
			return std::format("X: {}, Y: {}, Width: {}, Height: {}", position.x, position.y, size.width, size.height);
		}
	};

	static_assert(std::is_standard_layout_v<Rect2D<zI32>>, "Rect2D must be standard layout");
	static_assert(sizeof(Rect2D<zI32>) == 16, "Rect2D<zI32> must be 16 bytes");
}
