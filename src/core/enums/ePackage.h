#pragma once

#include "core/utils/Types.h"

namespace zzz::core
{
	enum class ePackage : zU32
	{
		ProjectManifest = 1,
		Scene = 2,
		Prefab = 4,
		BinaryAsset = 5,
		PrimaryView = 6,
		ChildView = 7,
		IndependentView = 8
	};
}
