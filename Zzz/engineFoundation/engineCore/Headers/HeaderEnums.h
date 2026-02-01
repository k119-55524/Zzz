#pragma once

namespace zzz
{
	// Битовая маска кнопок мыши
	enum class MouseButtonMask : zU8
	{
		None = 0,
		Left = 1 << 0,
		Right = 1 << 1,
		Middle = 1 << 2,
		Button4 = 1 << 3,
		Button5 = 1 << 4
	};

	inline MouseButtonMask operator|(MouseButtonMask a, MouseButtonMask b) { return static_cast<MouseButtonMask>(static_cast<zU8>(a) | static_cast<zU8>(b)); }
	inline MouseButtonMask& operator|=(MouseButtonMask& a, MouseButtonMask b) { a = a | b; return a; }
	inline MouseButtonMask operator&(MouseButtonMask a, MouseButtonMask b) { return static_cast<MouseButtonMask>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b)); }

	enum eInitState : zU32
	{
		InitNot,		// Готов к инициализации
		InitProcess,	// Идёт процесс инициализации
		InitOK,			// Инициализирован
		InitError,		// Ошибка инициализации
		Termination,	// Процесс деинициализации
		Running			// Идёт процесс работы
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
		Ratio_1x1,
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
		case eAspectType::Ratio_1x1:
			return 1.0f;
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
}