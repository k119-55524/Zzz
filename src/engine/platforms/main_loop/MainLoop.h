#pragma once
#include "MainLoop_Common.h"

#if Z_EDITOR
#include "MainLoop_Editor.h"
namespace zzz::engine { using MainLoop = MainLoop_Editor; } // Пустая заглушка
#elif Z_WINDOWS
#include "MainLoop_MSWin.h"
namespace zzz::engine { using MainLoop = MainLoop_MSWin; }
#elif Z_LINUX
#include "MainLoop_Linux.h"
namespace zzz::engine { using MainLoop = MainLoop_Linux; }
#elif Z_ANDROID
#include "MainLoop_Android.h"
namespace zzz::engine { using MainLoop = MainLoop_Android; }
#elif Z_MACOS
#include "MainLoop_MacOS.h"
namespace zzz::engine { using MainLoop = MainLoop_MacOS; }
#elif Z_IOS
#include "MainLoop_iOS.h"
namespace zzz::engine { using MainLoop = MainLoop_iOS; }
#else
#error ">>>>> MainLoop: Unsupported platform."
#endif

