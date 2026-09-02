#pragma once

#include "core/utils/Defines.h"

#if defined(Z_LINUX)

#include "core/hardware/MotherboardInfo.h"

namespace zzz::engine
{
	class MotherboardInfoCollectorLinux final
	{
	public:
		[[nodiscard]] ::zzz::core::MotherboardInfo Collect() const;
	};
}

#endif // defined(Z_LINUX)
