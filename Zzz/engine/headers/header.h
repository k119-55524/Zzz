#ifndef HEADER_H
#define HEADER_H

/* -------------------------------------------------------------

	Кастомные макросы которые используются в проекте:
		1. `ZTEST` - используется для включения тестовых проверок и сообщений.
		2. `ZLOG` - используется для логирования сообщений.

   ------------------------------------------------------------- */

#include "MSWindows.h"

namespace zzz
{
	typedef uint8_t zU8;
	typedef uint32_t zU32;
	typedef uint64_t zU64;

	typedef int8_t zI8;
	typedef int32_t zI32;
	typedef int64_t zI64;

	typedef float zF32;
	typedef double zF64;
}

#endif // HEADER_H
