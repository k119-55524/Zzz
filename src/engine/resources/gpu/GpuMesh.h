#pragma once

#if defined(Z_VULKAN)
#include "engine/resources/gpu/vulkan/GpuMesh_VK.h"
namespace zzz::engine
{
	using GpuMesh = GpuMesh_VK;
}
#elif defined(Z_D3D12)
#include "engine/resources/gpu/directx12/GpuMesh_DX.h"
namespace zzz::engine
{
	using GpuMesh = GpuMesh_DX;
}
#elif defined(Z_METAL)
#include "engine/resources/gpu/metal/GpuMesh_Metal.h"
namespace zzz::engine
{
	using GpuMesh = GpuMesh_Metal;
}
#else
#error "No graphics API defined for GpuMesh!"
#endif
