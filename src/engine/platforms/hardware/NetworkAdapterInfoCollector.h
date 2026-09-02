#pragma once
#include "core/utils/Defines.h"

#include "platforms/network/NetworkAdapterInfoCollectorEditor.h"
#include "platforms/network/NetworkAdapterInfoCollectorMSWin.h"
#include "platforms/network/NetworkAdapterInfoCollectorLinux.h"
#include "platforms/network/NetworkAdapterInfoCollectorMacOS.h"
#include "platforms/network/NetworkAdapterInfoCollectorMobile.h"

namespace zzz::engine
{
#if defined(Z_EDITOR)
	using NetworkAdapterInfoCollector = NetworkAdapterInfoCollectorEditor;
#elif defined(Z_WINDOWS)
	using NetworkAdapterInfoCollector = NetworkAdapterInfoCollectorMSWin;
#elif defined(Z_LINUX)
	using NetworkAdapterInfoCollector = NetworkAdapterInfoCollectorLinux;
#elif defined(Z_MACOS)
	using NetworkAdapterInfoCollector = NetworkAdapterInfoCollectorMacOS;
#elif defined(Z_MOBILE)
	using NetworkAdapterInfoCollector = NetworkAdapterInfoCollectorMobile;
#else
	#error ">>>>> NetworkAdapterInfoCollector: Unsupported platform."
#endif
}
