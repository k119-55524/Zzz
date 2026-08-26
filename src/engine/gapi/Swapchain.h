#pragma once

#include "engine/gapi/vulkan/Swapchain_VK.h"
#include "engine/gapi/metal/Swapchain_Metal.h"
#include "engine/gapi/directx12/Swapchain_DX.h"

namespace zzz::engine
{
#if defined(Z_D3D12)
	using Swapchain = Swapchain_DX;
#elif defined(Z_VULKAN)
	using Swapchain = Swapchain_VK;
#elif defined(Z_METAL)
	using Swapchain = Swapchain_Metal;
#else
#error "No graphics API defined for Swapchain!"
#endif
}