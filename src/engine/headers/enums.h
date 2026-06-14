#pragma once

#include <common/common.h>

namespace zzz
{
	enum class eInitState : zU8
	{
		NotInitialized,	// Готов к инициализации
		Initializing,	// Процесс инициализации
		Initialized,	// Инициализирован
		Running,		// Идёт процесс работы
		Destroying		// Процесс уничтожения
	};
}