#pragma once

#include "math/MathIncludes.h"
#include "math/vector/Vec2.h"
#include "math/vector/Vec3.h"
#include "math/vector/Vec4.h"
#include "math/utils/color/Color.h"

namespace zzz::core
{
	class Serializer;
}

namespace zzz::math
{
	/**
	 * @struct Vertex3D
	 * @brief Каноническая POD-структура вершины для 3D геометрии (Lit PBR / Normal Mapping).
	 *
	 * @details Макет памяти оптимизирован под кэш-линию CPU (ровно 64 байта) и 16-байтовое выравнивание SIMD/GPU:
	 *          - position: 12 байт (Vec3f)
	 *          - normal:   12 байт (Vec3f)
	 *          - texCoord:  8 байт (Vec2f)
	 *          - color:    16 байт (Color4f)
	 *          - tangent:  16 байт (Vec4f)
	 *          Итого: 64 байта, Standard Layout POD.
	 */
	struct alignas(16) Vertex3D
	{
		Vec3f   position{ 0.0f, 0.0f, 0.0f };       ///< 12б: Позиция в локальном пространстве (X, Y, Z)
		Vec3f   normal{ 0.0f, 0.0f, 1.0f };         ///< 12б: Вектор нормали поверхности (NX, NY, NZ)
		Vec2f        texCoord{ 0.0f, 0.0f };             ///< 8б:  Текстурные UV-координаты (U, V)
		Color4<zF32> color{ Palette4::White };           ///< 16б: Цвет вершины (RGBA [0..1])
		Vec4f        tangent{ 1.0f, 0.0f, 0.0f, 1.0f };  ///< 16б: Касательный вектор (XYZ + W знак бинормали)

		constexpr Vertex3D() noexcept = default;

		constexpr Vertex3D(
			const Vec3f& pos,
			const Vec3f& norm,
			const Vec2f& uv,
			const Color4<zF32>& col = Palette4::White,
			const Vec4f& tan = { 1.0f, 0.0f, 0.0f, 1.0f }) noexcept
			: position(pos)
			, normal(norm)
			, texCoord(uv)
			, color(col)
			, tangent(tan)
		{
		}

		[[nodiscard]] constexpr bool operator==(const Vertex3D&) const noexcept = default;

	private:
		friend class zzz::core::Serializer;
	};

	static_assert(sizeof(Vertex3D) == 64, "Vertex3D must be exactly 64 bytes (1 CPU cache line)");
	static_assert(alignof(Vertex3D) == 16, "Vertex3D must be 16-byte aligned");
	static_assert(std::is_standard_layout_v<Vertex3D>, "Vertex3D must be standard layout");
}
