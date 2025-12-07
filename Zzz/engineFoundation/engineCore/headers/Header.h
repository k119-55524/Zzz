#pragma once

#include <engineBase.h>
#include <logger.h>

namespace zzz
{
	enum class eProjType
	{
		Perspective,
		Orthographic
	};

	enum class eTypeWinResize : zU32
	{
		Show,
		Hide,
		Resize
	};

	enum class eShaderType : zU32
	{
		Vertex,     // Вершинный шейдер
		Pixel,      // Пиксельный шейдер (DirectX) / Фрагментный шейдер (OpenGL)
		Geometry,   // Геометрический шейдер
		Hull,       // Шейдер оболочки (Hull Shader) - управляющая стадия тесселяции
		Domain,     // Доменный шейдер (Domain Shader) - оценочная стадия тесселяции
		Compute     // Вычислительный шейдер
	};

	enum class eAspectType
	{
		// Стандартные
		Ratio_16x9,
		Ratio_9x16,
		Ratio_16x10,
		Ratio_10x16,
		Ratio_4x3,
		Ratio_3x4,

		// Ультра-широкие
		Ratio_21x9,
		Ratio_9x21,
		Ratio_32x9,
		Ratio_9x32,

		// Мобильные
		Ratio_18x9,
		Ratio_9x18,
		Ratio_20x9,
		Ratio_9x20,
		Ratio_19_5x9, // 19.5:9
		Ratio_9x19_5, // 9:19.5

		// Прочие
		Ratio_5x4,
		Ratio_4x5,

		Custom,
		FullWindow
	};

	//-----------------------------------
	// Общие константы для всех проектов
	//-----------------------------------
	const zU32 c_MinimumWindowsWidth = 150;		// TODO: Win 11 ограничение по минимальному размеру окна 148 при использовании системной рамки
	const zU32 c_MinimumWindowsHeight = 150;
	const zU32 c_MaximumWindowsWidth = 3840;	// Ultra HD 4K
	const zU32 c_MaximumWindowsHeight = 2160;	// Ultra HD 4K

	static constexpr zU32 BACK_BUFFER_COUNT = 2;
}