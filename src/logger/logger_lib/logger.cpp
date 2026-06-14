#include <iostream>

#include "logger.h"

namespace zzz::logger
{
	bool Logger::IsLogStreamEnabled = false;

	std::string Logger::MakeLogMessage(const std::source_location& loc, eLogMessageType type, const std::string& msg)
	{
		if (type == eLogMessageType::Message)
			return std::format(
				">>>>> [{}] {}{}",
				LogMessageTypeToString(type),
				msg,
				GetPlatformLogLineEnding());
		else
			return std::format(
				">>>>> [{}] {} -> line: {}, file: {}{}",
				LogMessageTypeToString(type),
				msg,
				loc.line(),
				loc.file_name(),
				GetPlatformLogLineEnding());
	}

	std::string Logger::MakeLogMessageError(const std::source_location& loc, eLogMessageType type, const std::string& msg)
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
#if Z_IDE_OUT_LOGS
#if defined(_MSC_VER)
		if (IsDebuggerPresent())
			OutputDebugStringA(output.c_str());
#elif Z_ANDROID
		__android_log_write(ANDROID_LOG_DEBUG, "Zzz", output.c_str());
#else
		std::cerr << output << std::endl;
#endif
#endif
	}
}
