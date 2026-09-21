#pragma once

#if defined(Z_VULKAN)
#include "engine/resources/gpu/vulkan/GpuMaterial_VK.h"
namespace zzz::engine
{
	using GpuMaterial = GpuMaterial_VK;
}
#elif defined(Z_D3D12)
#include "engine/resources/gpu/directx12/GpuMaterial_DX.h"
namespace zzz::engine
{
	using GpuMaterial = GpuMaterial_DX;
}
#elif defined(Z_METAL)
#include "engine/resources/gpu/metal/GpuMaterial_Metal.h"
namespace zzz::engine
{
	using GpuMaterial = GpuMaterial_Metal;
}
#else
#error "No graphics API defined for GpuMaterial!"
#endif
