#pragma once

#include "core/utils/Defines.h"

#if defined(Z_APPLE)

#include "core/hardware/RamInfo.h"

namespace zzz::engine
{
	// Общий коллектор для macOS и iOS: sysctl hw.memsize + host_statistics64 доступны на обеих платформах.
	class RamInfoCollectorApple final
	{
	public:
		[[nodiscard]] ::zzz::core::RamInfo Collect() const;
	};
}

#endif // defined(Z_APPLE)
