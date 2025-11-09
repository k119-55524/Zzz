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

	enum eTypeWinResize : zU32
	{
		Show,
		Hide,
		Resize
	};

	enum class eShaderType : zU32
	{
		Vertex,		// Вершинный шейдер
		Pixel,		// Пиксельный шейдер (ранее назывался фрагментный)
		Geometry,	// Геометрический шейдер
		Hull,		// Халловский шейдер (шейдер корпуса)
		Domain,		// Домейн шейдер (шейдер области)
		Compute		// Вычислительный шейдер
	};

	enum class eAspectType
	{
		Ratio_16_9,
		Ratio_16_10,
		Ratio_4_3,
		Ratio_21_9,
		Custom,
		FullWindow
	};

	//-----------------------------------
	// Общие константы для всех проектов
	//-----------------------------------
	const zU32 c_MinimumWindowsWidth = 100;
	const zU32 c_MinimumWindowsHeight = 100;
	const zU32 c_MaximumWindowsWidth = 3840;	// Ultra HD 4K
	const zU32 c_MaximumWindowsHeight = 2160;	// Ultra HD 4K

	static constexpr zU32 BACK_BUFFER_COUNT = 2;
}