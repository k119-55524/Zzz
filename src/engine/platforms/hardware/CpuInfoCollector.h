#pragma once
#include "core/utils/Defines.h"

#include "platforms/cpu/CpuInfoCollectorEditor.h"
#include "platforms/cpu/CpuInfoCollectorMSWin.h"
#include "platforms/cpu/CpuInfoCollectorLinux.h"
#include "platforms/cpu/CpuInfoCollectorApple.h"
#include "platforms/cpu/CpuInfoCollectorAndroid.h"

namespace zzz::engine
{
#if defined(Z_EDITOR)
	using CpuInfoCollector = CpuInfoCollectorEditor;
#elif defined(Z_WINDOWS)
	using CpuInfoCollector = CpuInfoCollectorMSWin;
#elif defined(Z_LINUX)
	using CpuInfoCollector = CpuInfoCollectorLinux;
#elif defined(Z_APPLE)
	using CpuInfoCollector = CpuInfoCollectorApple;
#elif defined(Z_ANDROID)
	using CpuInfoCollector = CpuInfoCollectorAndroid;
#else
	#error ">>>>> CpuInfoCollector: Unsupported platform."
#endif
}
