#pragma once

#include "core/utils/Defines.h"

#if defined(Z_APPLE)

#include <vector>
#include "core/hardware/CpuInfo.h"
#include "core/hardware/CpuTopology.h"

namespace zzz::engine
{
	// Общий коллектор для macOS и iOS: обе платформы используют один и тот же API sysctlbyname.
	class CpuInfoCollectorApple final
	{
	public:
		[[nodiscard]] std::vector<::zzz::core::CpuInfo> Collect() const;
		[[nodiscard]] ::zzz::core::CpuTopology CollectTopology() const;
	};
}

#endif // defined(Z_APPLE)
