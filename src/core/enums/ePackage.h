#pragma once

#include <core/utils/Types.h>

namespace zzz::core
{
	enum class ePackage : zU32
	{
		ProjectManifest = 1,
		Scene = 2,
		View = 3,
		Prefab = 4,
		BinaryAsset = 5,
		AppView = 6
	};
}
