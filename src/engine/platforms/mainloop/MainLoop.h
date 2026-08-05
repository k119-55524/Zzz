#pragma once
#include "MainLoopCommon.h"

#if Z_EDITOR
#include "MainLoopEditor.h"
namespace zzz::engine { using MainLoop = MainLoop_Editor; } // Пустая заглушка
#elif Z_WINDOWS
#include "MainLoopMSWin.h"
namespace zzz::engine { using MainLoop = MainLoop_MSWin; }
#elif Z_LINUX
#include "MainLoopLinux.h"
namespace zzz::engine { using MainLoop = MainLoop_Linux; }
#elif Z_ANDROID
#include "MainLoopAndroid.h"
namespace zzz::engine { using MainLoop = MainLoop_Android; }
#elif Z_MACOS
#include "MainLoopMacOS.h"
namespace zzz::engine { using MainLoop = MainLoop_MacOS; }
#elif Z_IOS
#include "MainLoopiOS.h"
namespace zzz::engine { using MainLoop = MainLoop_iOS; }
#else
#error ">>>>> MainLoop: Unsupported platform."
#endif

