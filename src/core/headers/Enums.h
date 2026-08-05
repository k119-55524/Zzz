#pragma once

#include "core/utils/Types.h"
#include "core/utils/Ensure.h"
#include "core/utils/Defines.h"
#include "core/utils/Macroses.h"
#include "core/utils/MemoryUtils.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz
{
	enum class eInitState : core::zU8
	{
		NotInitialized,	// Готов к инициализации
		Initializing,	// Процесс инициализации
		Initialized,	// Инициализирован
		Running,		// Идёт процесс работы
		Destroying		// Процесс уничтожения
	};
}
