#pragma once

#include "core/utils/Defines.h"

#if defined(Z_APPLE)
#include <mach-o/dyld.h>
#include <TargetConditionals.h>
#include <pthread.h>
#endif // defined(Z_APPLE)