#pragma once

#include "core/utils/Defines.h"

#if defined(Z_EDITOR)
#include "WinMSWindowEditor.h"
namespace zzz::engine { using NativeWindow = WinMSWindowEditor; }
#elif defined(Z_WINDOWS)
#include "WinMSWindows.h"
namespace zzz::engine { using NativeWindow = WinMSWindows; }
#elif defined(Z_LINUX)
#include "WinLinux.h"
namespace zzz::engine { using NativeWindow = WinLinux; }
#elif defined(Z_ANDROID)
#include "WinAndroid.h"
namespace zzz::engine { using NativeWindow = WinAndroid; }
#elif defined(Z_MACOS)
#include "WinMacOS.h"
namespace zzz::engine { using NativeWindow = WinMacOS; }
#elif defined(Z_IOS)
#include "WiniOS.h"
namespace zzz::engine { using NativeWindow = WiniOS; }
#else
#error ">>>>> NativeWindow: Unsupported platform."
#endif