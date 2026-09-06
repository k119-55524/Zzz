#pragma once

#include "math/utils/Types.h"

namespace zzz::core
{
	/// @brief Подкаталоги пользовательских данных (Path::GetDirectory()) - Read-Write, внутренние для Path.
	enum class eUserDirectoryKind : zU32
	{
		Cache = 1, // .../cache
		Saves = 2, // .../saves
		Logs  = 3  // .../logs
	};
}
