#pragma once

#include <string_view>
#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	enum class eWindowState : zU8
	{
		Normal,               // Обычное окно на рабочем столе
		Maximized,            // Развернуто на весь рабочий стол
		BorderlessFullscreen, // Полноэкранный режим без рамок
		ExclusiveFullscreen,  // Эксклюзивный полноэкранный режим
		Minimized,            // Свёрнуто на панель задач / в трей
		Closed                // Окно закрыто пользователем
	};

	constexpr std::string_view ToString(eWindowState state)
	{
		switch (state)
		{
		case eWindowState::Normal:               return "Normal";
		case eWindowState::Maximized:            return "Maximized";
		case eWindowState::BorderlessFullscreen: return "BorderlessFullscreen";
		case eWindowState::ExclusiveFullscreen:  return "ExclusiveFullscreen";
		case eWindowState::Minimized:            return "Minimized";
		case eWindowState::Closed:               return "Closed";
		}
		THROW_RUNTIME("Необработанный eWindowState");
	}
}
