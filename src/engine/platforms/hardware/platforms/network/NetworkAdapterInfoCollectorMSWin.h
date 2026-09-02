#pragma once

#include "core/utils/Defines.h"

#if defined(Z_WINDOWS)

#include <vector>
#include "core/hardware/NetworkAdapterInfo.h"

namespace zzz::engine
{
	class NetworkAdapterInfoCollectorMSWin final
	{
	public:
		[[nodiscard]] std::vector<::zzz::core::NetworkAdapterInfo> Collect() const;
	};
}

#endif // defined(Z_WINDOWS)
