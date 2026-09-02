#pragma once

#include "core/utils/Defines.h"

#if defined(Z_WINDOWS)

#include "core/hardware/MotherboardInfo.h"

namespace zzz::engine
{
	class MotherboardInfoCollectorMSWin final
	{
	public:
		[[nodiscard]] ::zzz::core::MotherboardInfo Collect() const;
	};
}

#endif // defined(Z_WINDOWS)
