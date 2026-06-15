#pragma once
#include "InputBase.h"

#if Z_WINDOWS
#include "InputMSWindows.h"
namespace zzz::engine { using Input = InputMSWindows; }
#elif Z_LINUX
#include "InputLinux.h"
namespace zzz::engine { using Input = InputLinux; }
#elif Z_ANDROID
#include "InputAndroid.h"
namespace zzz::engine { using Input = InputAndroid; }
#elif Z_MACOS
#include "InputMacOS.h"
namespace zzz::engine { using Input = InputMacOS; }
#elif Z_IOS
#include "InputiOS.h"
namespace zzz::engine { using Input = InputiOS; }
#else
#error ">>>>> zzz::engine::Input: Unsupported platform."
#endif
