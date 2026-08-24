#pragma once

#if defined(Z_D3D12)
#include "engine/gapi/directx12/Swapchain_DX.h"
namespace zzz::engine
{
	using Swapchain = Swapchain_DX;
}
#elif defined(Z_VULKAN)
#include "engine/gapi/vulkan/Swapchain_VK.h"
namespace zzz::engine
{
	using Swapchain = Swapchain_VK;
}
#elif defined(Z_METAL)
#include "engine/gapi/metal/Swapchain_Metal.h"
namespace zzz::engine
{
	using Swapchain = Swapchain_Metal;
}
#else
#error "No graphics API defined for Swapchain!"
#endif
