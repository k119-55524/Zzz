#pragma once

#include "core/utils/Defines.h"

#if defined(Z_LINUX) || defined(Z_MACOS) || defined(Z_MOBILE)

#include <vector>
#include "core/hardware/GpuInfo.h"

namespace zzz::engine
{
	/**
	 * @brief Единый честный GPU-стаб для Linux/macOS/мобильных платформ. Реальное перечисление GPU
	 * через Vulkan/Metal для целей телеметрии (не путать с выбором адаптера для рендера,
	 * см. gapi/selectors/gpu/) - задача соответствующих этапов платформенного роллаута.
	 */
	class GpuInfoCollectorStub final
	{
	public:
		[[nodiscard]] std::vector<::zzz::core::GpuInfo> Collect() const
		{
			using namespace zzz::core;
			std::vector<GpuInfo> gpus;
			gpus.emplace_back("unknown", "Unknown", 0u, 0u, eGPUType::Unknown, 0ull, 0ull, 0ull);
			return gpus;
		}
	};
}

#endif // defined(Z_LINUX) || defined(Z_MACOS) || defined(Z_MOBILE)
