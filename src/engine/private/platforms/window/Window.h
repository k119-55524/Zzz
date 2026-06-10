#pragma once

#if defined(Z_WINDOWS)
#include "WinMSWindows.h"
namespace zzz::engine { using Window = WinMSWindows; }
#elif defined(Z_LINUX)
#include "WinLinux.h"
namespace zzz::engine { using Window = WinLinux; }
#elif defined(Z_ANDROID)
#include "WinAndroid.h"
namespace zzz::engine { using Window = WinAndroid; }
#elif defined(Z_MACOS)
#include "WinMacOS.h"
namespace zzz::engine { using Window = WinMacOS; }
#elif defined(Z_IOS)
#include "WiniOS.h"
namespace zzz::engine { using Window = WiniOS; }
#else
#error "Unsupported platform for Window"
#endif
