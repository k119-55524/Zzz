#pragma once
#include "Input_Common.h"

#if defined(Z_WINDOWS)
#include "InputMSWindows.h"
namespace zzz::engine { using Input = InputMSWindows; }
#elif defined(Z_LINUX)
#include "InputLinux.h"
namespace zzz::engine { using Input = InputLinux; }
#elif defined(Z_ANDROID)
#include "InputAndroid.h"
namespace zzz::engine { using Input = InputAndroid; }
#elif defined(Z_MACOS)
#include "InputMacOS.h"
namespace zzz::engine { using Input = InputMacOS; }
#elif defined(Z_IOS)
#include "InputiOS.h"
namespace zzz::engine { using Input = InputiOS; }
#else
#error "Unsupported platform for Input"
#endif
