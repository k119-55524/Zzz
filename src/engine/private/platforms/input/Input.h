#pragma once
#include "Input_Common.h"

#if defined(Z_WINDOWS)
#include "platforms/InputMSWindows.h"
namespace zzz::engine { using Input = InputMSWindows; }
#elif defined(Z_LINUX)
#include "platforms/InputLinux.h"
namespace zzz::engine { using Input = InputLinux; }
#elif defined(Z_ANDROID)
#include "platforms/InputAndroid.h"
namespace zzz::engine { using Input = InputAndroid; }
#elif defined(Z_MACOS)
#include "platforms/InputMacOS.h"
namespace zzz::engine { using Input = InputMacOS; }
#elif defined(Z_IOS)
#include "platforms/InputiOS.h"
namespace zzz::engine { using Input = InputiOS; }
#else
#error "Unsupported platform for Input"
#endif
