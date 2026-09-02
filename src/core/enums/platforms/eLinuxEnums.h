#pragma once

#include <string_view>
#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	enum class eLinuxDisplayServer : zU8
	{
		Auto,            // Автоопределение (Wayland -> X11)
		Wayland,         // Нативный Wayland
		X11              // X11 сервер
	};

	constexpr std::string_view ToString(eLinuxDisplayServer type)
	{
		switch (type)
		{
		case eLinuxDisplayServer::Auto:   return "Auto";
		case eLinuxDisplayServer::Wayland:return "Wayland";
		case eLinuxDisplayServer::X11:    return "X11";
		}
		THROW_RUNTIME("Необработанный eLinuxDisplayServer");
	}

	enum class eLinuxWindowMode : zU8
	{
		Windowed,
		Fullscreen
	};

	constexpr std::string_view ToString(eLinuxWindowMode type)
	{
		switch (type)
		{
		case eLinuxWindowMode::Windowed:  return "Windowed";
		case eLinuxWindowMode::Fullscreen:return "Fullscreen";
		}
		THROW_RUNTIME("Необработанный eLinuxWindowMode");
	}
}
