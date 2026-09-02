#pragma once

#include "core/utils/Defines.h"

#if defined(Z_LINUX)

#include "core/hardware/RamInfo.h"

namespace zzz::engine
{
	class RamInfoCollectorLinux final
	{
	public:
		[[nodiscard]] ::zzz::core::RamInfo Collect() const;
	};
}

#endif // defined(Z_LINUX)
