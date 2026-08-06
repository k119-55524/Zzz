#pragma once

#include "engine/gapi/IGAPI.h"

#if defined(Z_VULKAN)
#include "vulkan/VulkanAPI.h"
namespace zzz::engine
{
	using GAPI = VulkanAPI;
}
#elif defined(Z_D3D12)
#include "directx12/DirectX12API.h"
namespace zzz::engine
{
	using GAPI = DirectX12API;
}
#elif defined(Z_METAL)
#include "metal/MetalAPI.h"
namespace zzz::engine
{
	using GAPI = MetalAPI;
}
#else
#error "No graphics API defined for the current platform!"
#endif
