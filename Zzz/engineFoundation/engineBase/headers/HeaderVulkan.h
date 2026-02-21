
#pragma once

#if defined(ZRENDER_API_VULKAN)
#if (_DEBUG)
#pragma comment(lib, "volkd.lib")
#else
#pragma comment(lib, "volk.lib")
#endif

#if defined(ZPLATFORM_MSWINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(ZPLATFORM_ANDROID)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(ZPLATFORM_LINUX)
#if defined(USE_WAYLAND)
#define VK_USE_PLATFORM_WAYLAND_KHR
#else
#define VK_USE_PLATFORM_XCB_KHR
#endif // USE_WAYLAND
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif // #if defined(ZPLATFORM_MSWINDOWS)

// ---- Vulkan ----
#pragma warning(push)
#pragma warning(disable: 28251)
#include <volk/volk.h>
#pragma warning(pop)

#include <vulkan/vulkan_win32.h>

// ---- GLM ----
#define GLM_FORCE_RADIANS
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>

static constexpr DXGI_FORMAT BACK_BUFFER_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
static constexpr DXGI_FORMAT DEPTH_FORMAT = DXGI_FORMAT_D32_FLOAT;

#endif // ZRENDER_API_VULKAN
