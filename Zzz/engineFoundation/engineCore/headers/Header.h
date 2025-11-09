#pragma once

#include <engineBase.h>
#include <logger.h>

namespace zzz
{
	enum eTypeWinResize : zU32
	{
		eShow,
		eHide,
		eResize
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

	//-----------------------------------
	// Общие константы для всех проектов
	//-----------------------------------
	const zU32 c_MinimumWindowsWidth = 100;
	const zU32 c_MinimumWindowsHeight = 100;
	const zU32 c_MaximumWindowsWidth = 3840;	// Ultra HD 4K
	const zU32 c_MaximumWindowsHeight = 2160;	// Ultra HD 4K

	static constexpr zU32 BACK_BUFFER_COUNT = 2;
}