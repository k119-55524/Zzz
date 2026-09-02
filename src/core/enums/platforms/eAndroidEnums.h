#pragma once

#include <string_view>
#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	enum class eAndroidScreenOrientation : zU8
	{
		Sensor,          // Автоматический поворот по сенсору
		Portrait,        // Портретная (вертикальная)
		LandscapeLeft,   // Горизонтальная (кнопка слева)
		LandscapeRight   // Горизонтальная (кнопка справа)
	};

	constexpr std::string_view ToString(eAndroidScreenOrientation type)
	{
		switch (type)
		{
		case eAndroidScreenOrientation::Sensor:        return "Sensor";
		case eAndroidScreenOrientation::Portrait:      return "Portrait";
		case eAndroidScreenOrientation::LandscapeLeft: return "LandscapeLeft";
		case eAndroidScreenOrientation::LandscapeRight:return "LandscapeRight";
		}
		THROW_RUNTIME("Необработанный eAndroidScreenOrientation");
	}

	enum class eAndroidCutoutMode : zU8
	{
		Default,         // Системный режим по умолчанию
		ShortEdges,      // Заходить рендерингом под вырез/чёлку
		Never            // Не заходить под вырез (чёрная полоса)
	};

	constexpr std::string_view ToString(eAndroidCutoutMode type)
	{
		switch (type)
		{
		case eAndroidCutoutMode::Default:   return "Default";
		case eAndroidCutoutMode::ShortEdges:return "ShortEdges";
		case eAndroidCutoutMode::Never:     return "Never";
		}
		THROW_RUNTIME("Необработанный eAndroidCutoutMode");
	}
}
