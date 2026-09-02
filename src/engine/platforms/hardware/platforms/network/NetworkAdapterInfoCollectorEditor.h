#pragma once

#include "core/utils/Defines.h"

#if defined(Z_EDITOR)

#include <vector>
#include "core/hardware/NetworkAdapterInfo.h"

namespace zzz::engine
{
	// См. CpuInfoCollectorEditor.h. Прежний PlatformEditor.cpp не передавал networkAdapters вовсе
	// (пустой список по умолчанию в HardwareState) - поведение сохранено как есть.
	class NetworkAdapterInfoCollectorEditor final
	{
	public:
		[[nodiscard]] std::vector<::zzz::core::NetworkAdapterInfo> Collect() const
		{
			return {};
		}
	};
}

#endif // defined(Z_EDITOR)
