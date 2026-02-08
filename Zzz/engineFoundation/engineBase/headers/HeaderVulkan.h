
#pragma once

#if defined(ZRENDER_API_VULKAN)
#if (_DEBUG)
#pragma comment(lib, "volkd.lib")
#else
#pragma comment(lib, "volk.lib")
#endif

// ---- Vulkan ----
#include <Volk/volk.h>
#include <vulkan/vulkan_win32.h>

// ---- GLM ----
#define GLM_FORCE_RADIANS
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>

#endif // ZRENDER_API_VULKAN
