#include "logger.h"

namespace zzz::logger
{
	std::string Logger::MakeLogMessage(const std::source_location& loc, eLogMessageType type, const std::string& msg)
	{
		return std::format(
			">>>>> [{}] {} -> line: {}, file: {}{}",
			LogMessageTypeToString(type),
			msg,
			loc.line(),
			loc.file_name(),
			GetPlatformLogLineEnding());
	}

	std::string Logger::MakeLogMessageCritical(const std::source_location& loc, eLogMessageType type, const std::string& msg)
	{
		return std::format(
			">>>>> [{}] {} -> [{}]. line: {}, file: {}{}",
			LogMessageTypeToString(type),
			msg,
			loc.function_name(),
			loc.line(),
			loc.file_name(),
			GetPlatformLogLineEnding());
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
