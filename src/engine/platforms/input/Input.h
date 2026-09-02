#pragma once

#include "core/utils/Defines.h"

#if defined(Z_EDITOR)
#include "InputBase.h"
#include "InputEditor.h"
namespace zzz::engine { using Input = InputEditor; }
#elif defined(Z_WINDOWS)
#include "InputBase.h"
#include "InputMSWindows.h"
namespace zzz::engine { using Input = InputMSWindows; }
#elif defined(Z_LINUX)
#include "InputBase.h"
#include "InputLinux.h"
namespace zzz::engine { using Input = InputLinux; }
#elif defined(Z_ANDROID)
#include "InputBase.h"
#include "InputAndroid.h"
namespace zzz::engine { using Input = InputAndroid; }
#elif defined(Z_MACOS)
#include "InputBase.h"
#include "InputMacOS.h"
namespace zzz::engine { using Input = InputMacOS; }
#elif defined(Z_IOS)
#include "InputBase.h"
#include "InputiOS.h"
namespace zzz::engine { using Input = InputiOS; }
#else
#error ">>>>> Input: Unsupported platform."
#endif
