#pragma once

#include "core/utils/Types.h"

namespace zzz::core
{
	/// @brief Каталоги ассетов (Read-Only, рядом с исполняемым файлом) - переиспользуется вне Path.
	enum class eAssetDirectoryKind : zU32
	{
		Textures = 1,
		Video    = 2,
		Audio    = 3,
		Fonts    = 4
	};
}
