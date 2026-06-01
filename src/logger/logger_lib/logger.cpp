#include "logger.h"

namespace zzz::logger
{
	std::string Logger::MakeLogMessage(const std::source_location& loc, eLogMessageType type, const std::string& msg)
	{
		auto end = "\n";

		// На Linux и Android перенос строки в логах не нужен, так как они уже добавляются автоматически.
#if defined(__linux__)
		end = "";
#endif

		return std::format(
			">>>>> [{}] {} -> line: {}, file: {}{}",
			LogMessageTypeToString(type),
			msg,
			loc.line(),
			loc.file_name(),
			end);
	}

	std::string Logger::MakeLogMessageFatal(const std::source_location& loc, eLogMessageType type, const std::string& msg)
	{
		return std::format(
			">>>>> [{}] {} -> [{}]. line: {}, file: {}\n",
			LogMessageTypeToString(type),
			msg,
			loc.function_name(),
			loc.line(),
			loc.file_name());
	}

	constexpr const char* Logger::LogMessageTypeToString(eLogMessageType type)
	{
		switch (type)
		{
		case eLogMessageType::Message:		return "MESSAGE";
		case eLogMessageType::Warning:		return "WARNING";
		case eLogMessageType::Error:		return "ERROR";
		case eLogMessageType::Exception:	return "EXCEPTION";
		case eLogMessageType::Fatal:		return "FATAL";
		}

		std::unreachable();
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
