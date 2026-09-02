#pragma once

#include "core/utils/Defines.h"

#if defined(Z_EDITOR)

#include "core/hardware/MotherboardInfo.h"

namespace zzz::engine
{
	// См. CpuInfoCollectorEditor.h - фейковые данные Editor сохранены как есть (stage_06, п. 3.5.2).
	class MotherboardInfoCollectorEditor final
	{
	public:
		[[nodiscard]] ::zzz::core::MotherboardInfo Collect() const
		{
			using namespace zzz::core;
			return MotherboardInfo("Editor Vendor", "Editor Model", "00000000-0000-0000-0000-000000000000");
		}
	};
}

#endif // defined(Z_EDITOR)
