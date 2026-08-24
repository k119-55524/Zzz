#pragma once

#include "vulkan/SurfView_VK.h"
#include "directx12/SurfView_DX.h"
#include "metal/SurfView_Metal.h"
#include "engine/gapi/ISurfView.h"

namespace zzz::engine
{
#if defined(Z_D3D12)
	using SurfView = SurfView_DX;
#elif defined(Z_VULKAN)
	using SurfView = SurfView_VK;
#elif defined(Z_METAL)
	using SurfView = SurfView_Metal;
#else
#error "No graphics API defined for SurfView!"
#endif
}