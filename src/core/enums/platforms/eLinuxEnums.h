#pragma once

#include <common/Defines.h>

namespace zzz::common
{
	enum class eLinuxDisplayServer : zU8
	{
		Auto,            // Автоопределение (Wayland -> X11)
		Wayland,         // Нативный Wayland
		X11              // X11 сервер
	};

	enum class eLinuxWindowMode : zU8
	{
		Windowed,
		Fullscreen
	};
}
