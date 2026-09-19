
#pragma once

#if defined(Z_VULKAN)

#if defined(Z_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(Z_ANDROID)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(Z_LINUX)
#if defined(USE_WAYLAND)
#define VK_USE_PLATFORM_WAYLAND_KHR
#else
#define VK_USE_PLATFORM_XCB_KHR
#endif // USE_WAYLAND
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif // #if defined(Z_WINDOWS)

// ---- Vulkan ----
#pragma warning(push)
#pragma warning(disable: 28251 26110 26495 6386 6387 26813)
#include <vulkan/vulkan.h>
#pragma warning(pop)

// ---- GLM ----
#define GLM_FORCE_RADIANS
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>

#endif // Z_VULKAN