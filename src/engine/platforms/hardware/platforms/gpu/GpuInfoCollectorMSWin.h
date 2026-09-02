#pragma once

#include "core/utils/Defines.h"

#if defined(Z_WINDOWS)

#include <vector>
#include "core/hardware/GpuInfo.h"

namespace zzz::engine
{
	class GpuInfoCollectorMSWin final
	{
	public:
		[[nodiscard]] std::vector<::zzz::core::GpuInfo> Collect() const;
	};
}

#endif // defined(Z_WINDOWS)
