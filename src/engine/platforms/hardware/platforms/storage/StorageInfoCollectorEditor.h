#pragma once

#include "core/utils/Defines.h"

#if defined(Z_EDITOR)

#include <vector>
#include "core/hardware/StorageInfo.h"

namespace zzz::engine
{
	// См. CpuInfoCollectorEditor.h. Прежний PlatformEditor.cpp не передавал storages вовсе (пустой список
	// по умолчанию в HardwareState) - поведение сохранено как есть.
	class StorageInfoCollectorEditor final
	{
	public:
		[[nodiscard]] std::vector<::zzz::core::StorageInfo> Collect() const
		{
			return {};
		}
	};
}

#endif // defined(Z_EDITOR)
