#ifndef HEADER_H
#define HEADER_H

/* -------------------------------------------------------------

	 астомные макросы которые используютс€ в проекте:
		1. `ZTEST` - используетс€ дл€ включени€ тестовых проверок и сообщений.
		2. `ZLOG` - используетс€ дл€ логировани€ сообщений.

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
	// ќбщие константы дл€ всех типов проектов
	//------------------------------------------

	const zU64 c_MinimumWindowsWidth = 100;
	const zU64 c_MinimumWindowsHeight = 100;
	const zU64 c_MaximumWindowsWidth = 3840;	// Ultra HD 4K
	const zU64 c_MaximumWindowsHeight = 2160;	// Ultra HD 4K

	[[noreturn]]
	inline void throw_runtime_error(
		const std::string& msg = "Throw runtime error",
		const std::source_location& loc = std::source_location::current())
	{
		throw std::runtime_error(
			"Exception: " + msg +
			". Method=" + std::string(loc.function_name()) +
			", line=" + std::to_string(loc.line()) +
			", file=" + std::string(loc.file_name()));
	}

	template<typename T>
	inline void ensure(T&& condition,
		const std::string& msg = "Ensure failed",
		const std::source_location& loc = std::source_location::current())
	{
		if (!condition)
			throw std::runtime_error(
				"Ensure: " + msg +
				". Method=" + std::string(loc.function_name()) +
				", line=" + std::to_string(loc.line()) +
				", file=" + std::string(loc.file_name()));
	}

	inline void DebugOutput(const std::wstring& msg)
	{
#if defined(_WIN64) && defined(_DEBUG)
		OutputDebugStringW(msg.c_str());
#elif defined(_DEBUG)
		std::wcerr << msg << std::endl;
#else
		// ¬ релизной сборке ничего не делаем :)
#endif
	}
}
#endif // HEADER_H