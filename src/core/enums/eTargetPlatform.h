#pragma once

#include <string_view>
#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	enum class eTargetPlatform : zU8
	{
		Windows,
		Linux,
		Android,
		MacOS,
		iOS
	};

	constexpr std::string_view ToString(eTargetPlatform type)
	{
		switch (type)
		{
		case eTargetPlatform::Windows: return "Windows";
		case eTargetPlatform::Linux:   return "Linux";
		case eTargetPlatform::Android: return "Android";
		case eTargetPlatform::MacOS:   return "MacOS";
		case eTargetPlatform::iOS:     return "iOS";
		}
		THROW_RUNTIME("Необработанный eTargetPlatform");
	}
}
