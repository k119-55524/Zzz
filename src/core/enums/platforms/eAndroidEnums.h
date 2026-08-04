#pragma once

namespace zzz::core
{
	enum class eAndroidScreenOrientation : zU8
	{
		Sensor,          // Автоматический поворот по сенсору
		Portrait,        // Портретная (вертикальная)
		LandscapeLeft,   // Горизонтальная (кнопка слева)
		LandscapeRight   // Горизонтальная (кнопка справа)
	};

	enum class eAndroidCutoutMode : zU8
	{
		Default,         // Системный режим по умолчанию
		ShortEdges,      // Заходить рендерингом под вырез/чёлку
		Never            // Не заходить под вырез (чёрная полоса)
	};
}
