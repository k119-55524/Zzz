#pragma once

#include <foundation.h>

namespace zzz
{
	enum class eInitState : zU8
	{
		InitNot,		// Готов к инициализации
		InitOK,			// Инициализирован
		Running			// Идёт процесс работы
	};
}