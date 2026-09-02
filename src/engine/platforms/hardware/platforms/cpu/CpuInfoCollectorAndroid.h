#pragma once

#include "core/utils/Defines.h"

#if defined(Z_ANDROID)

#include <vector>
#include "core/hardware/CpuInfo.h"

namespace zzz::engine
{
	class CpuInfoCollectorAndroid final
	{
	public:
		[[nodiscard]] std::vector<::zzz::core::CpuInfo> Collect() const;
	};
}

#endif // defined(Z_ANDROID)
