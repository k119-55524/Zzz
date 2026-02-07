
#pragma once

#if defined(ZRENDER_API_VULKAN)

// ---- Vulkan ----
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

// ---- GLM ----
#define GLM_FORCE_RADIANS
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>

#endif // ZRENDER_API_VULKAN
