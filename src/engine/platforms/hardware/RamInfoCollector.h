#pragma once
#include "core/utils/Defines.h"

#include "platforms/ram/RamInfoCollectorEditor.h"
#include "platforms/ram/RamInfoCollectorMSWin.h"
#include "platforms/ram/RamInfoCollectorLinux.h"
#include "platforms/ram/RamInfoCollectorApple.h"
#include "platforms/ram/RamInfoCollectorAndroid.h"

namespace zzz::engine
{
#if defined(Z_EDITOR)
	using RamInfoCollector = RamInfoCollectorEditor;
#elif defined(Z_WINDOWS)
	using RamInfoCollector = RamInfoCollectorMSWin;
#elif defined(Z_LINUX)
	using RamInfoCollector = RamInfoCollectorLinux;
#elif defined(Z_APPLE)
	using RamInfoCollector = RamInfoCollectorApple;
#elif defined(Z_ANDROID)
	using RamInfoCollector = RamInfoCollectorAndroid;
#else
	#error ">>>>> RamInfoCollector: Unsupported platform."
#endif
}
