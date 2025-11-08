#pragma once

#include <memory>
#include <string>
#include <cstdint>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <source_location>

#include "PlatformsDefines.h"
#include "HeadersMSWin.h"
#include "HeaderVulkan.h"
#include "HeaderMetal.h"
#include "headerDX.h"
#include "MacroDefinition.h"

namespace zzz
{
	typedef uint8_t zU8;
	typedef uint16_t zU16;
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

	[[noreturn]]
	inline void throw_invalid_argument(
		const std::string& msg = "Throw invalid argument",
		const std::source_location& loc = std::source_location::current())
	{
		throw std::invalid_argument(
			"Exception: " + msg +
			". Method=" + std::string(loc.function_name()) +
			", line=" + std::to_string(loc.line()) +
			", file=" + std::string(loc.file_name()));
	}

	[[noreturn]]
	inline void throw_out_of_range(
		const std::string& msg = "Throw out of range",
		const std::source_location& loc = std::source_location::current())
	{
		throw std::out_of_range(
			"Exception: " + msg +
			". Method=" + std::string(loc.function_name()) +
			", line=" + std::to_string(loc.line()) +
			", file=" + std::string(loc.file_name()));
	}

	// Математическое переполнение
	[[noreturn]]
	inline void throw_overflow_error(
		const std::string& msg = "Throw overflow error",
		const std::source_location& loc = std::source_location::current())
	{
		throw std::overflow_error(
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
#if defined(_DEBUG)
#if defined(ZPLATFORM_MSWINDOWS)
		OutputDebugStringW((msg + L"\n").c_str());
#else
		std::wcerr << msg << std::endl;
#endif	// ZPLATFORM_MSWINDOWS
#endif	// _DEBUG
	}
}