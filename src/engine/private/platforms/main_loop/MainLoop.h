#pragma once
#include "MainLoop_Common.h"

#if defined(Z_WINDOWS)
#include "MainLoop_MSWin.h"
namespace zzz::engine { using MainLoop = MainLoop_MSWin; }
#elif defined(Z_LINUX)
#include "MainLoop_Linux.h"
namespace zzz::engine { using MainLoop = MainLoop_Linux; }
#elif defined(Z_ANDROID)
#include "MainLoop_Android.h"
namespace zzz::engine { using MainLoop = MainLoop_Android; }
#elif defined(Z_MACOS)
#include "MainLoop_MacOS.h"
namespace zzz::engine { using MainLoop = MainLoop_MacOS; }
#elif defined(Z_IOS)
#include "MainLoop_iOS.h"
namespace zzz::engine { using MainLoop = MainLoop_iOS; }
#else
#error "Unsupported platform for MainLoop"
#endif
