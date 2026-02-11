
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

// Максимальная поддерживаемая версия Vulkan, которую мы будем использовать.
// Это не означает, что она будет поддерживаться на всех устройствах,
// но мы будем стараться использовать её возможности, если они доступны.
constexpr uint32_t VULKAN_ENGINE_MAX_VERSION = VK_API_VERSION_1_4;

#endif // ZRENDER_API_VULKAN
