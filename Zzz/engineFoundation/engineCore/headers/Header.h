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

	constexpr inline zF32 GetAspect(eAspectType aspectType, zF32 width = 0, zF32 height = 0)
	{
		switch (aspectType)
		{
			// Стандартные
		case eAspectType::Ratio_16x9:
			return 16.0f / 9.0f;
		case eAspectType::Ratio_9x16:
			return 9.0f / 16.0f;
		case eAspectType::Ratio_16x10:
			return 16.0f / 10.0f;
		case eAspectType::Ratio_10x16:
			return 10.0f / 16.0f;
		case eAspectType::Ratio_4x3:
			return 4.0f / 3.0f;
		case eAspectType::Ratio_3x4:
			return 3.0f / 4.0f;

			// Ультра-широкие
		case eAspectType::Ratio_21x9:
			return 21.0f / 9.0f;
		case eAspectType::Ratio_9x21:
			return 9.0f / 21.0f;
		case eAspectType::Ratio_32x9:
			return 32.0f / 9.0f;
		case eAspectType::Ratio_9x32:
			return 9.0f / 32.0f;

			// Мобильные
		case eAspectType::Ratio_18x9:
			return 18.0f / 9.0f;
		case eAspectType::Ratio_9x18:
			return 9.0f / 18.0f;
		case eAspectType::Ratio_20x9:
			return 20.0f / 9.0f;
		case eAspectType::Ratio_9x20:
			return 9.0f / 20.0f;
		case eAspectType::Ratio_19_5x9:
			return 19.5f / 9.0f;
		case eAspectType::Ratio_9x19_5:
			return 9.0f / 19.5f;

			// Прочие
		case eAspectType::Ratio_5x4:
			return 5.0f / 4.0f;
		case eAspectType::Ratio_4x5:
			return 4.0f / 5.0f;

		case eAspectType::Custom:
		case eAspectType::FullWindow:
			if (height <= 0.0f)
				throw_runtime_error(">>>>> [GetAspect( ... )]. Height must be greater than zero for Custom/FullWindow aspect.");
			return width / height;

		default:
			throw_runtime_error(">>>>> [GetAspect( ... )]. Invalid aspect type.");
		}
	}

	//-----------------------------------
	// Общие константы для всех проектов
	//-----------------------------------
	const zU32 c_MinimumWindowsWidth = 150;		// TODO: Win 11 ограничение по минимальному размеру окна 148 при использовании системной рамки
	const zU32 c_MinimumWindowsHeight = 150;
	const zU32 c_MaximumWindowsWidth = 3840;	// Ultra HD 4K
	const zU32 c_MaximumWindowsHeight = 2160;	// Ultra HD 4K

	static constexpr zU32 BACK_BUFFER_COUNT = 2;
}