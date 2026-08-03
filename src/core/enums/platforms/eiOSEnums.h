#pragma once

#include <common/Defines.h>

namespace zzz::common
{
	enum class eiOSScreenOrientation : zU8
	{
		AutoRotate,      // Автоматический поворот
		Portrait,        // Портретная (вертикальная)
		LandscapeLeft,   // Горизонтальная влево
		LandscapeRight   // Горизонтальная вправо
	};

	enum class eiOSSafeAreaMode : zU8
	{
		UseSafeArea,        // Использовать Safe Area (черные поля под челку)
		ExtendIntoSafeArea  // Рендерить во весь экран (заходить под челку/Dynamic Island)
	};

	enum class eiOSHomeIndicatorMode : zU8
	{
		Visible,            // Полоса "Домой" всегда видна
		AutoHidden          // Автоматически скрывать полосу "Домой"
	};
}
