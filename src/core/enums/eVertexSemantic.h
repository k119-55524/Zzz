#pragma once

#include "math/utils/Types.h"

namespace zzz::core
{
	/**
	 * @enum eVertexSemantic
	 * @brief Семантическое назначение входного канала (атрибута) вершины для GPU и CPU.
	 */
	enum class eVertexSemantic : zU8
	{
		Position,       ///< Позиция вершины в локальном пространстве (X, Y, Z)
		Normal,         ///< Нормаль поверхности (NX, NY, NZ)
		TexCoord,       ///< Текстурные UV-координаты (U, V)
		Color,          ///< Цвет вершины (RGBA [0..1])
		Tangent,        ///< Касательный вектор (XYZ + W знак бинормали)
		Bitangent,      ///< Бикасательный вектор / Бинормаль
		BlendWeight,    ///< Веса костей для скелетной анимации
		BlendIndices,   ///< Индексы костей для скелетной анимации
		Count           ///< Общее количество поддерживаемых семантик
	};
}
