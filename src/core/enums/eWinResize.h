#pragma once

#include <string_view>
#include "core/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	enum class eWinResize : zU32
	{
		Show,
		Hide,
		Resize
	};

	constexpr std::string_view ToString(eWinResize type)
	{
		switch (type)
		{
		case eWinResize::Show:   return "SHOW";
		case eWinResize::Hide:   return "HIDE";
		case eWinResize::Resize: return "RESIZE";
		}
		THROW_RUNTIME("Необработанный eWinResize");
	}
}
