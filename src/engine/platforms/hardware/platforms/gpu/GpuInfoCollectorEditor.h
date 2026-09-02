#pragma once

#include "core/utils/Defines.h"

#if defined(Z_EDITOR)

#include <vector>
#include "core/hardware/GpuInfo.h"

namespace zzz::engine
{
	// См. CpuInfoCollectorEditor.h - фейковые данные Editor сохранены как есть (stage_06, п. 3.5.2).
	class GpuInfoCollectorEditor final
	{
	public:
		[[nodiscard]] std::vector<::zzz::core::GpuInfo> Collect() const
		{
			using namespace zzz::core;
			return { GpuInfo("editor_gpu_id", "Editor GPU", 0, 0, eGPUType::Integrated, 1024 * 1024 * 1024, 0, 0) };
		}
	};
}

#endif // defined(Z_EDITOR)
