#pragma once

#include <engineBase.h>
#include <logger.h>

namespace zzz
{
	enum class eTopology : zU32
	{
		Undefined = 0,
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip,

		// Расширенные (не все поддерживаются везде)
		//TriangleFan,			// Только Vulkan
		//LineListAdjacency,		// DX12, Vulkan
		//LineStripAdjacency,		// DX12, Vulkan
		//TriangleListAdjacency,	// DX12, Vulkan
		//TriangleStripAdjacency,// DX12, Vulkan
		//PatchList				// Vulkan (тесселяция)
	};

	enum class eSurfClearType : zU32
	{
		None = 0,	// Без очистки
		Color = 1	// Очистка фона цветом
	};

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

	//-----------------------------------
	// Общие константы для всех проектов
	//-----------------------------------
	const zU32 c_MinimumWindowsWidth = 150;		// TODO: Win 11 ограничение по минимальному размеру окна 148 при использовании системной рамки
	const zU32 c_MinimumWindowsHeight = 150;
	const zU32 c_MaximumWindowsWidth = 3840;	// Ultra HD 4K
	const zU32 c_MaximumWindowsHeight = 2160;	// Ultra HD 4K

	static constexpr zU32 BACK_BUFFER_COUNT = 2;
}