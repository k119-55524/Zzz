#pragma once
#include "core/utils/Defines.h"

#include "platforms/storage/StorageInfoCollectorEditor.h"
#include "platforms/storage/StorageInfoCollectorMSWin.h"
#include "platforms/storage/StorageInfoCollectorLinux.h"
#include "platforms/storage/StorageInfoCollectorMacOS.h"
#include "platforms/storage/StorageInfoCollectorMobile.h"

namespace zzz::engine
{
#if defined(Z_EDITOR)
	using StorageInfoCollector = StorageInfoCollectorEditor;
#elif defined(Z_WINDOWS)
	using StorageInfoCollector = StorageInfoCollectorMSWin;
#elif defined(Z_LINUX)
	using StorageInfoCollector = StorageInfoCollectorLinux;
#elif defined(Z_MACOS)
	using StorageInfoCollector = StorageInfoCollectorMacOS;
#elif defined(Z_MOBILE)
	using StorageInfoCollector = StorageInfoCollectorMobile;
#else
	#error ">>>>> StorageInfoCollector: Unsupported platform."
#endif
}
