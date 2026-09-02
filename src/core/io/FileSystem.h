#pragma once

#include "core/utils/Defines.h"
#include "FileSystemBase.h"

#if Z_WINDOWS || Z_LINUX || Z_MACOS
#include "platforms/FileSystemDesktop.h"
namespace zzz::core { using FileSystem = FileSystemDesktop; }
#elif Z_ANDROID
#include "platforms/FileSystemAndroid.h"
namespace zzz::core { using FileSystem = FileSystemAndroid; }
#elif Z_IOS
#include "platforms/FileSystemiOS.h"
namespace zzz::core { using FileSystem = FileSystemiOS; }
#else
#error ">>>>> FileSystem: Unsupported platform."
#endif
