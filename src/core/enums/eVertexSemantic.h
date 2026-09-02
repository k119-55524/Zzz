#pragma once

#include <string_view>
#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

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

	constexpr std::string_view ToString(eVertexSemantic semantic)
	{
		switch (semantic)
		{
		case eVertexSemantic::Position:     return "Position";
		case eVertexSemantic::Normal:       return "Normal";
		case eVertexSemantic::TexCoord:     return "TexCoord";
		case eVertexSemantic::Color:        return "Color";
		case eVertexSemantic::Tangent:      return "Tangent";
		case eVertexSemantic::Bitangent:    return "Bitangent";
		case eVertexSemantic::BlendWeight:  return "BlendWeight";
		case eVertexSemantic::BlendIndices: return "BlendIndices";
		case eVertexSemantic::Count:        return "Count";
		}
		THROW_RUNTIME("Необработанный eVertexSemantic");
	}
}
