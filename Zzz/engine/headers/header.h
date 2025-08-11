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

	enum e_TypeWinResize : zU32
	{
		eShow,
		eHide,
		eResize
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

	template<typename T>
	inline void SafeRelease(T*& ptr)
	{
		if (ptr)
		{
			ptr->Release();
			ptr = nullptr;
		}
	}

#pragma region SafeMake
	template<typename Creator, typename... Args>
	inline decltype(auto) safe_make_impl(
		Creator&& creator,
		const std::source_location& loc,
		Args&&... args) noexcept(false)
	{
		try
		{
			return creator(std::forward<Args>(args)...);
		}
		catch (const std::bad_alloc& e)
		{
			std::ostringstream oss;
			oss << "Memory allocation error. Exception: " << e.what()
				<< ". Method=" << loc.function_name()
				<< ", line=" << loc.line()
				<< ", file=" << loc.file_name();
			throw std::runtime_error(oss.str());
		}
	}

	// Безопасный make_shared
	template<typename T, typename... Args>
	inline std::shared_ptr<T> safe_make_shared(Args&&... args) noexcept(false)
	{
		return safe_make_impl(
			[](auto&&... a) { return std::make_shared<T>(std::forward<decltype(a)>(a)...); },
			std::source_location::current(),
			std::forward<Args>(args)...
		);
	}

	// Безопасный make_unique
	template<typename T, typename... Args>
	inline std::unique_ptr<T> safe_make_unique(Args&&... args) noexcept(false)
	{
		return safe_make_impl(
			[](auto&&... a) { return std::make_unique<T>(std::forward<decltype(a)>(a)...); },
			std::source_location::current(),
			std::forward<Args>(args)...
		);
	}
#pragma endregion SafeMake

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
}
#endif // HEADER_H