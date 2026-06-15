#pragma once

#if Z_WINDOWS
#include "WinMSWindows.h"
namespace zzz::engine { using Window = WinMSWindows; }
#elif Z_LINUX
#include "WinLinux.h"
namespace zzz::engine { using Window = WinLinux; }
#elif Z_ANDROID
#include "WinAndroid.h"
namespace zzz::engine { using Window = WinAndroid; }
#elif Z_MACOS
#include "WinMacOS.h"
namespace zzz::engine { using Window = WinMacOS; }
#elif Z_IOS
#include "WiniOS.h"
namespace zzz::engine { using Window = WiniOS; }
#else
#error ">>>>> Window: Unsupported platform."
#endif

