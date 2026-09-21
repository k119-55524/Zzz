#pragma once

#if defined(Z_VULKAN)
#include "engine/resources/gpu/vulkan/GpuShader_VK.h"
namespace zzz::engine
{
	using GpuShader = GpuShader_VK;
}
#elif defined(Z_D3D12)
#include "engine/resources/gpu/directx12/GpuShader_DX.h"
namespace zzz::engine
{
	using GpuShader = GpuShader_DX;
}
#elif defined(Z_METAL)
#include "engine/resources/gpu/metal/GpuShader_Metal.h"
namespace zzz::engine
{
	using GpuShader = GpuShader_Metal;
}
#else
#error "No graphics API defined for GpuShader!"
#endif
