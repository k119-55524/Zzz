#include <iostream>

#include "logger.h"

namespace zzz::logger
{
	std::wstring Logger::MakeDebugOutputString(const std::source_location& loc, const std::wstring& msg)
	{
		return
			L">>>>> -= DebugOutput =-\n"
			L"    Message: " + msg +
			L"\n    Source: [" +
			std::wstring(loc.function_name(),
				loc.function_name() + std::strlen(loc.function_name())) +
			L"] line: " + std::to_wstring(loc.line()) +
			L", file: " +
			std::wstring(loc.file_name(),
				loc.file_name() + std::strlen(loc.file_name())) +
			L"\n";
	}

	void Logger::DebugOutputRaw(const std::wstring& output) noexcept
	{
#if defined(_MSC_VER)
		if (IsDebuggerPresent())
			OutputDebugStringW(output.c_str());
#elif defined(__ANDROID__)
		// Simple narrowing for Android logcat.
		// For full Unicode support, a proper UTF-8 conversion should be used.
		std::string narrow(output.begin(), output.end());
		__android_log_write(ANDROID_LOG_DEBUG, "Zzz", narrow.c_str());
#else // For non-MSVC compilers, we can write to standard error as a fallback.
		std::wcerr << output << std::endl;
#endif // #if defined(_MSC_VER)
	}
}
