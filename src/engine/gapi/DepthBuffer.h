#pragma once

#if defined(Z_D3D12)
#include "engine/gapi/directx12/DepthBuffer_DX.h"
namespace zzz::engine
{
	using DepthBuffer = DepthBuffer_DX;
}
#elif defined(Z_VULKAN)
#include "engine/gapi/vulkan/DepthBuffer_VK.h"
namespace zzz::engine
{
	using DepthBuffer = DepthBuffer_VK;
}
#elif defined(Z_METAL)
#include "engine/gapi/metal/DepthBuffer_Metal.h"
namespace zzz::engine
{
	using DepthBuffer = DepthBuffer_Metal;
}
#else
#error "No graphics API defined for DepthBuffer!"
#endif
