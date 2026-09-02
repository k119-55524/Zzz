#pragma once

#include "core/utils/Defines.h"

#if defined(Z_EDITOR)

#include "core/hardware/RamInfo.h"

namespace zzz::engine
{
	// См. CpuInfoCollectorEditor.h - фейковые данные Editor сохранены как есть (stage_06, п. 3.5.2).
	class RamInfoCollectorEditor final
	{
	public:
		[[nodiscard]] ::zzz::core::RamInfo Collect() const
		{
			using namespace zzz::core;
			return RamInfo(1024 * 1024 * 1024, 512 * 1024 * 1024);
		}
	};
}

#endif // defined(Z_EDITOR)
