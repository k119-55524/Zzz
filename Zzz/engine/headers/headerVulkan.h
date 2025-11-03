#pragma once

#if defined(ZRENDER_API_VULKAN)
#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#endif // defined(ZRENDER_API_VULKAN)