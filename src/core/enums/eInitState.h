#pragma once

#include "core/utils/Types.h"
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
