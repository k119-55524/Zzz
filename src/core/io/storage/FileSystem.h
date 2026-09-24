#pragma once

#include "core/utils/Defines.h"

#if defined(Z_DESKTOP)
#include "platforms/FileSystemDesktop.h"
namespace zzz::core { using FileSystem = FileSystemDesktop; }
#elif defined(Z_ANDROID)
#include "platforms/FileSystemAndroid.h"
namespace zzz::core { using FileSystem = FileSystemAndroid; }
#elif defined(Z_IOS)
#include "platforms/FileSystemiOS.h"
namespace zzz::core { using FileSystem = FileSystemiOS; }
#else
#error ">>>>> FileSystem: Unsupported platform."
#endif
