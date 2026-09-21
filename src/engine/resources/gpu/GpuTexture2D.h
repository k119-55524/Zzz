#pragma once

#if defined(Z_VULKAN)
#include "engine/resources/gpu/vulkan/GpuTexture2D_VK.h"
namespace zzz::engine
{
	using GpuTexture2D = GpuTexture2D_VK;
}
#elif defined(Z_D3D12)
#include "engine/resources/gpu/directx12/GpuTexture2D_DX.h"
namespace zzz::engine
{
	using GpuTexture2D = GpuTexture2D_DX;
}
#elif defined(Z_METAL)
#include "engine/resources/gpu/metal/GpuTexture2D_Metal.h"
namespace zzz::engine
{
	using GpuTexture2D = GpuTexture2D_Metal;
}
#else
#error "No graphics API defined for GpuTexture2D!"
#endif
