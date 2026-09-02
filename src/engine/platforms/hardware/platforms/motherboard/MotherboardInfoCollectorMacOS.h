#pragma once

#include "core/utils/Defines.h"

#if defined(Z_MACOS)

#include "core/hardware/MotherboardInfo.h"

namespace zzz::engine
{
	class MotherboardInfoCollectorMacOS final
	{
	public:
		[[nodiscard]] ::zzz::core::MotherboardInfo Collect() const;
	};
}

#endif // defined(Z_MACOS)
