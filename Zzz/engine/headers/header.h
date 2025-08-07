#ifndef HEADER_H
#define HEADER_H

/* -------------------------------------------------------------

	Кастомные макросы которые используются в проекте:
		1. `ZTEST` - используется для включения тестовых проверок и сообщений.
		2. `ZLOG` - используется для логирования сообщений.

   ------------------------------------------------------------- */

#include "headerMSWin.h"
#include "headerDX.h"

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

	enum eInitState : zU32
	{
		eInitNot,		// Готов к инициализации
		eInitProcess,	// Идёт процесс инициализации
		eInitOK,		// Инициализирован
		eInitError,		// Ошибка инициализации
		eTermination,	// Процесс деинициализации
		eRunning		// Идёт процесс работы
	};

	//------------------------------------------
	// Общие константы для всех типов проектов
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
				msg +
				". Method=" + std::string(loc.function_name()) +
				", line=" + std::to_string(loc.line()) +
				", file=" + std::string(loc.file_name()));
	}

	template<typename T, typename... Args>
	inline std::shared_ptr<T> safe_make_shared(
		Args&&... args)
	{
		return safe_make_shared_impl<T>(std::source_location::current(), std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	inline std::shared_ptr<T> safe_make_shared_impl(
		const std::source_location& loc,
		Args&&... args)
	{
		try {
			return std::make_shared<T>(std::forward<Args>(args)...);
		}
		catch (const std::bad_alloc& e) {
			throw std::runtime_error(
				"Memory allocation error (make_shared). Exception: " + std::string(e.what()) +
				". Method=" + std::string(loc.function_name()) +
				", line=" + std::to_string(loc.line()) +
				", file=" + std::string(loc.file_name()));
		}
	}

	template<typename T, typename... Args>
	inline std::unique_ptr<T> safe_make_unique(
		Args&&... args)
	{
		return safe_make_unique_impl<T>(std::source_location::current(), std::forward<Args>(args)...);
	}

	// Внутренняя реализация с source_location
	template<typename T, typename... Args>
	inline std::unique_ptr<T> safe_make_unique_impl(
		const std::source_location& loc,
		Args&&... args)
	{
		try {
			return std::make_unique<T>(std::forward<Args>(args)...);
		}
		catch (const std::bad_alloc& e) {
			throw std::runtime_error(
				"Memory allocation error (make_unique). Exception: " + std::string(e.what()) +
				". Method=" + std::string(loc.function_name()) +
				", line=" + std::to_string(loc.line()) +
				", file=" + std::string(loc.file_name()));
		}
	}

	inline void DebugOutput(const std::wstring& msg)
	{
#if defined(_WIN64) && defined(_DEBUG)
		OutputDebugStringW(msg.c_str());
#elif defined(_DEBUG)
		std::wcerr << msg << std::endl;
#else
		// В релизной сборке ничего не делаем :)
#endif
	}

#define RELEASE(x) if ((x)) { (x)->Release(); (x) = nullptr; }
}
#endif // HEADER_H