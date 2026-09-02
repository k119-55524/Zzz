#pragma once
#include "core/utils/Defines.h"

#include "platforms/gpu/GpuInfoCollectorEditor.h"
#include "platforms/gpu/GpuInfoCollectorMSWin.h"
#include "platforms/gpu/GpuInfoCollectorStub.h"

namespace zzz::engine
{
#if defined(Z_EDITOR)
	using GpuInfoCollector = GpuInfoCollectorEditor;
#elif defined(Z_WINDOWS)
	using GpuInfoCollector = GpuInfoCollectorMSWin;
#elif defined(Z_LINUX) || defined(Z_MACOS) || defined(Z_MOBILE)
	using GpuInfoCollector = GpuInfoCollectorStub;
#else
	#error ">>>>> GpuInfoCollector: Unsupported platform."
#endif
}
