#pragma once

#include "Types/Types.h"
#include "Helpers/zVersion.h"

namespace Zzz
{
	const zU64 c_MinimumWindowsWidth = 100;
	const zU64 c_MinimumWindowsHeight = 100;
	const zU64 c_MaximumWindowsWidth = 3840;	// Ultra HD 4K
	const zU64 c_MaximumWindowsHeight = 2160;	// Ultra HD 4K

	const zStr c_AppResourcesFolder(L"data");

	const zVersion c_VerScenesSerialize(0, 0, 1, 1);
	const zStr c_HeapSceneFile(L"hzsf");
}