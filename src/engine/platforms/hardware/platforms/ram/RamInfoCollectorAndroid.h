#pragma once

#include "core/utils/Defines.h"

#if defined(Z_ANDROID)

#include "core/hardware/RamInfo.h"

namespace zzz::engine
{
	class RamInfoCollectorAndroid final
	{
	public:
		[[nodiscard]] ::zzz::core::RamInfo Collect() const;
	};
}

#endif // defined(Z_ANDROID)
