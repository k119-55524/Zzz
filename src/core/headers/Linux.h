#pragma once

#include "core/utils/Defines.h"

#if defined(Z_LINUX)
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include <sys/resource.h>
#include <wayland-client.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#endif // defined(Z_LINUX)