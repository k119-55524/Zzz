#pragma once

#include "ztypes.h"

namespace zzz
{
	enum eInitState : zU32
	{
		InitNot,		// Готов к инициализации
		InitProcess,	// Идёт процесс инициализации
		InitOK,			// Инициализирован
		InitError,		// Ошибка инициализации
		Termination,	// Процесс деинициализации
		Running			// Идёт процесс работы
	};
}