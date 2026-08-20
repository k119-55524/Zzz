#pragma once

#include "core/utils/Types.h"

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
}
