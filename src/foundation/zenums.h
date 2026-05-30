#pragma once

#include "ztypes.h"

namespace zzz
{
	enum class eInitState : zU8
	{
		InitNot,		// Готов к инициализации
		InitProcess,	// Идёт процесс инициализации
		InitOK,			// Инициализирован
		InitError,		// Ошибка инициализации
		Termination,	// Процесс деинициализации
		Running			// Идёт процесс работы
	};

	enum class eLogMessageType : zU8
	{
		Message,
		Warning,
		Error,
		Exception
	};
}