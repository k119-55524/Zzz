#pragma once

#include <string_view>
#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	enum class eiOSScreenOrientation : zU8
	{
		AutoRotate,      // Автоматический поворот
		Portrait,        // Портретная (вертикальная)
		LandscapeLeft,   // Горизонтальная влево
		LandscapeRight   // Горизонтальная вправо
	};

	constexpr std::string_view ToString(eiOSScreenOrientation type)
	{
		switch (type)
		{
		case eiOSScreenOrientation::AutoRotate:    return "AutoRotate";
		case eiOSScreenOrientation::Portrait:      return "Portrait";
		case eiOSScreenOrientation::LandscapeLeft: return "LandscapeLeft";
		case eiOSScreenOrientation::LandscapeRight:return "LandscapeRight";
		}
		THROW_RUNTIME("Необработанный eiOSScreenOrientation");
	}

	enum class eiOSSafeAreaMode : zU8
	{
		UseSafeArea,        // Использовать Safe Area (черные поля под челку)
		ExtendIntoSafeArea  // Рендерить во весь экран (заходить под челку/Dynamic Island)
	};

	constexpr std::string_view ToString(eiOSSafeAreaMode type)
	{
		switch (type)
		{
		case eiOSSafeAreaMode::UseSafeArea:       return "UseSafeArea";
		case eiOSSafeAreaMode::ExtendIntoSafeArea: return "ExtendIntoSafeArea";
		}
		THROW_RUNTIME("Необработанный eiOSSafeAreaMode");
	}

	enum class eiOSHomeIndicatorMode : zU8
	{
		Visible,            // Полоса "Домой" всегда видна
		AutoHidden          // Автоматически скрывать полосу "Домой"
	};

	constexpr std::string_view ToString(eiOSHomeIndicatorMode type)
	{
		switch (type)
		{
		case eiOSHomeIndicatorMode::Visible:   return "Visible";
		case eiOSHomeIndicatorMode::AutoHidden:return "AutoHidden";
		}
		THROW_RUNTIME("Необработанный eiOSHomeIndicatorMode");
	}
}
