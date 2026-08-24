#pragma once

#include "core/utils/Types.h"

namespace zzz::core
{
	enum class ePackage : zU32
	{
		ProjectManifest = 1,
		Scene           = 2,
		PrimaryView     = 3,
		ChildView       = 4,
		IndependentView = 5,
		Prefab          = 6,
		BinaryAsset     = 7
	};
}
