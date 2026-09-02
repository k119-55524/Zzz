#pragma once

#include "core/utils/Defines.h"

#if defined(Z_MACOS)

#include <vector>
#include "core/hardware/NetworkAdapterInfo.h"

namespace zzz::engine
{
	class NetworkAdapterInfoCollectorMacOS final
	{
	public:
		[[nodiscard]] std::vector<::zzz::core::NetworkAdapterInfo> Collect() const;
	};
}

#endif // defined(Z_MACOS)
