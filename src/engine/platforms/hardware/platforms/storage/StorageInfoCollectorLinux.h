#pragma once

#include "core/utils/Defines.h"

#if defined(Z_LINUX)

#include <vector>
#include "core/hardware/StorageInfo.h"

namespace zzz::engine
{
	class StorageInfoCollectorLinux final
	{
	public:
		[[nodiscard]] std::vector<::zzz::core::StorageInfo> Collect() const;
	};
}

#endif // defined(Z_LINUX)
