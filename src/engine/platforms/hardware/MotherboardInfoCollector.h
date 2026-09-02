#pragma once

#include "core/utils/Defines.h"
#include "platforms/motherboard/MotherboardInfoCollectorMSWin.h"
#include "platforms/motherboard/MotherboardInfoCollectorLinux.h"
#include "platforms/motherboard/MotherboardInfoCollectorMacOS.h"
#include "platforms/motherboard/MotherboardInfoCollectorMobile.h"
#include "platforms/motherboard/MotherboardInfoCollectorEditor.h"

namespace zzz::engine
{
#if defined(Z_EDITOR)
	using MotherboardInfoCollector = MotherboardInfoCollectorEditor;
#elif defined(Z_WINDOWS)
	using MotherboardInfoCollector = MotherboardInfoCollectorMSWin;
#elif defined(Z_LINUX)
	using MotherboardInfoCollector = MotherboardInfoCollectorLinux;
#elif defined(Z_MACOS)
	using MotherboardInfoCollector = MotherboardInfoCollectorMacOS;
#elif defined(Z_MOBILE)
	using MotherboardInfoCollector = MotherboardInfoCollectorMobile;
#else
	#error ">>>>> MotherboardInfoCollector: Unsupported platform."
#endif
}
