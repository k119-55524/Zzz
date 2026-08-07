#pragma once

#if Z_EDITOR
#include "WinMSWindowEditor.h"
namespace zzz::engine { using NativeWindow = WinMSWindowEditor; }
#elif Z_WINDOWS
#include "WinMSWindows.h"
namespace zzz::engine { using NativeWindow = WinMSWindows; }
#elif Z_LINUX
#include "WinLinux.h"
namespace zzz::engine { using NativeWindow = WinLinux; }
#elif Z_ANDROID
#include "WinAndroid.h"
namespace zzz::engine { using NativeWindow = WinAndroid; }
#elif Z_MACOS
#include "WinMacOS.h"
namespace zzz::engine { using NativeWindow = WinMacOS; }
#elif Z_IOS
#include "WiniOS.h"
namespace zzz::engine { using NativeWindow = WiniOS; }
#else
#error ">>>>> NativeWindow: Unsupported platform."
#endif