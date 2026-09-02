#pragma once

#include "core/utils/Defines.h"

#if defined(Z_LINUX)

#include <vector>
#include "core/hardware/CpuInfo.h"

namespace zzz::engine
{
	class CpuInfoCollectorLinux final
	{
	public:
		[[nodiscard]] std::vector<::zzz::core::CpuInfo> Collect() const;
	};
}

#endif // defined(Z_LINUX)
