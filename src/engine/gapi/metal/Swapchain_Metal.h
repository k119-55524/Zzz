#pragma once

#include "engine/gapi/clear_config/SurfaceClearConfig.h"

#if defined(Z_METAL)

namespace zzz::engine
{
	class Swapchain_Metal final
	{
	public:
		Swapchain_Metal() = default;
		~Swapchain_Metal() = default;

		void SetClearConfig(const SurfaceClearConfig& config) { (void)config; }
	};
}

#endif // Z_METAL
