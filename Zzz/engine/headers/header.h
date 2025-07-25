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

	//------------------------------------------
	// Общие константы для всех типов проектов
	//------------------------------------------

	const zU64 c_MinimumWindowsWidth = 100;
	const zU64 c_MinimumWindowsHeight = 100;
	const zU64 c_MaximumWindowsWidth = 3840;	// Ultra HD 4K
	const zU64 c_MaximumWindowsHeight = 2160;	// Ultra HD 4K
}

#define ZERO_STRUCT(structure) { std::memset(&(structure), 0, sizeof(structure)); }

#define THROW_RUNTIME_ERROR(msg) {										\
	throw std::runtime_error(											\
		std::string(">>>>> --- ERROR that caused the EXCEPTION ---") +	\
		"\n+--- " + msg +												\
		"\n+--- Function: " + __FUNCTION__ +							\
		"\n+--- File: " + __FILE__ +									\
		"\n+--- Line: " + std::to_string(__LINE__) +					\
		"\n-------------------------------------\n"						\
	);																	\
}

#define ZASSERT(param) {								\
	if (param) {										\
			throw std::runtime_error(					\
			std::string(">>>>> --- Assert: true ---") + \
			"\n+--- Function: " + __FUNCTION__ +		\
			"\n+--- File: " + __FILE__ +				\
			"\n+--- Line: " + std::to_string(__LINE__) +\
			"\n-------------------------------------\n"	\
		);												\
	} \
}

#define ZASSERT_NULLPTR(param) {						\
	if (param == nullptr) {								\
		throw std::runtime_error(						\
			std::string(">>>>> --- Null pointer ---") +	\
			"\n+--- Function: " + __FUNCTION__ +		\
			"\n+--- File: " + __FILE__ +				\
			"\n+--- Line: " + std::to_string(__LINE__) +\
			"\n-------------------------------------\n"	\
		);												\
	} \
}

#endif // HEADER_H
