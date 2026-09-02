#pragma once

#include "core/utils/Defines.h"

#if defined(Z_EDITOR)
#include "MainLoopCommon.h"
#include "MainLoopEditor.h"
namespace zzz::engine { using MainLoop = MainLoop_Editor; } // Пустая заглушка
#elif defined(Z_WINDOWS)
#include "MainLoopCommon.h"
#include "MainLoopMSWin.h"
namespace zzz::engine { using MainLoop = MainLoop_MSWin; }
#elif defined(Z_LINUX)
#include "MainLoopCommon.h"
#include "MainLoopLinux.h"
namespace zzz::engine { using MainLoop = MainLoop_Linux; }
#elif defined(Z_ANDROID)
#include "MainLoopCommon.h"
#include "MainLoopAndroid.h"
namespace zzz::engine { using MainLoop = MainLoop_Android; }
#elif defined(Z_MACOS)
#include "MainLoopCommon.h"
#include "MainLoopMacOS.h"
namespace zzz::engine { using MainLoop = MainLoop_MacOS; }
#elif defined(Z_IOS)
#include "MainLoopCommon.h"
#include "MainLoopiOS.h"
namespace zzz::engine { using MainLoop = MainLoop_iOS; }
#else
#error ">>>>> MainLoop: Unsupported platform."
#endif

