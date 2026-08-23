#pragma once

#include "engine/gapi/ISurfView.h"

#if defined(Z_VULKAN)
#include "vulkan/SurfView_VK.h"
namespace zzz::engine
{
	using SurfView = SurfView_VK;
}
#elif defined(Z_D3D12)
#include "directx12/SurfView_DX.h"
namespace zzz::engine
{
	using SurfView = SurfView_DX;
}
#elif defined(Z_METAL)
#include "metal/SurfView_Metal.h"
namespace zzz::engine
{
	using SurfView = SurfView_Metal;
}
#else
#error "No graphics API defined for SurfView!"
#endif
