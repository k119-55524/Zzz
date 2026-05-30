#pragma once

#include "../../foundation/zenums.h"
//#include <zmacros.h>

#include "header.h"

using namespace zzz;

namespace zzz::logger
{
	class Logger
	{
	public:
		template<typename... Args>
		static void LogMessage(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
		{
#if ZADD_LOGGER
			auto formatted = std::format(fmt, std::forward<Args>(args)...);
			auto output = MakeLogMessage(loc, eLogMessageType::Message, formatted);
			DebugOutputIDE(output);
#endif
		}

		template<typename... Args>
		static void LogWarning(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
		{
#if ZADD_LOGGER
			auto formatted = std::format(fmt, std::forward<Args>(args)...);
			auto output = MakeLogMessage(loc, eLogMessageType::Warning, formatted);
			DebugOutputIDE(output);
#endif
		}

		template<typename... Args>
		static void LogError(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
		{
#if ZADD_LOGGER
			auto formatted = std::format(fmt, std::forward<Args>(args)...);
			auto output = MakeLogMessage(loc, eLogMessageType::Error, formatted);
			DebugOutputIDE(output);
#endif
		}

		template<typename... Args>
		static void LogException(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
		{
#if ZADD_LOGGER
			auto formatted = std::format(fmt, std::forward<Args>(args)...);
			auto output = MakeLogMessage(loc, eLogMessageType::Exception, formatted);
			DebugOutputIDE(output);
#endif
		}

	private:
		static std::string MakeLogMessage(const std::source_location& loc, eLogMessageType type, const std::string& msg);
		static constexpr const char* LogMessageTypeToString(eLogMessageType type);

		static void DebugOutputIDE(const std::string& output) noexcept;
	};
}
