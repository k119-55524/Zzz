#pragma once

#include "core/utils/Defines.h"

#if defined(Z_WINDOWS)

#include <vector>
#include "core/hardware/StorageInfo.h"

namespace zzz::engine
{
	class StorageInfoCollectorMSWin final
	{
	public:
		[[nodiscard]] std::vector<::zzz::core::StorageInfo> Collect() const;
	};
}

#endif // defined(Z_WINDOWS)
