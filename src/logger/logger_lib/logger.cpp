#include <iostream>

#include "logger.h"

namespace zzz::logger
{
	std::string Logger::MakeDebugOutputString(const std::source_location& loc, const std::string& msg)
	{
		return
			">>>>> -= DebugOutput =-\n"
			"    Message: " + msg +
			"\n    Source: [" +
			std::string(loc.function_name()) +
			"] line: " + std::to_string(loc.line()) +
			", file: " +
			std::string(loc.file_name()) +
			"\n";
	}

	std::string Logger::MakeDebugOutputLiteString(const std::source_location& loc, const std::string& msg)
	{
		return
			">>>>> " + msg + " -> [" +
			std::string(loc.function_name()) +
			"] line: " + std::to_string(loc.line()) +
			", file: " +
			std::string(loc.file_name()) +
			"\n";
	}

	void Logger::DebugOutputIDE(const std::string& output) noexcept
	{
#if ZIDE_OUT_LOGS
#if defined(_MSC_VER)
		if (IsDebuggerPresent())
			OutputDebugStringA(output.c_str());
#elif defined(__ANDROID__)
		__android_log_write(ANDROID_LOG_DEBUG, "Zzz", output.c_str());
#else
		std::cerr << output << std::endl;
#endif
#endif
	}
}
