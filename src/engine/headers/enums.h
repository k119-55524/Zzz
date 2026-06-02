#pragma once

#include <foundation.h>

namespace zzz
{
	enum class eInitState : zU8
	{
		NotInitialized,	// Готов к инициализации
		Initialized,	// Инициализирован
		Running,		// Идёт процесс работы
		Destroying,		// Процесс уничтожения
	};
}