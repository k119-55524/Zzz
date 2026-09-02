#pragma once

#include <string_view>
#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	enum class eMacOSWindowMode : zU8
	{
		Windowed,
		Fullscreen
	};

	constexpr std::string_view ToString(eMacOSWindowMode type)
	{
		switch (type)
		{
		case eMacOSWindowMode::Windowed:  return "Windowed";
		case eMacOSWindowMode::Fullscreen:return "Fullscreen";
		}
		THROW_RUNTIME("Необработанный eMacOSWindowMode");
	}
}
