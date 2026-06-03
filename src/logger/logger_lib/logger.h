#pragma once

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
#if Z_ADD_LOGGER
			auto formatted = std::format(fmt, std::forward<Args>(args)...);
			auto output = MakeLogMessage(loc, eLogMessageType::Message, formatted);
			DebugOutputIDE(output);
#endif
		}

		template<typename... Args>
		static void LogWarning(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
		{
#if Z_ADD_LOGGER
			auto formatted = std::format(fmt, std::forward<Args>(args)...);
			auto output = MakeLogMessage(loc, eLogMessageType::Warning, formatted);
			DebugOutputIDE(output);
#endif
		}

		template<typename... Args>
		static void LogError(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
		{
#if Z_ADD_LOGGER
			auto formatted = std::format(fmt, std::forward<Args>(args)...);
			auto output = MakeLogMessageError(loc, eLogMessageType::Error, formatted);
			DebugOutputIDE(output);
#endif
		}

		template<typename... Args>
		static void LogException(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
		{
#if Z_ADD_LOGGER
			auto formatted = std::format(fmt, std::forward<Args>(args)...);
			auto output = MakeLogMessageError(loc, eLogMessageType::Exception, formatted);
			DebugOutputIDE(output);
#endif
		}

		template<typename... Args>
		static void LogCritical(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
		{
			auto formatted = std::format(fmt, std::forward<Args>(args)...);
			auto output = MakeLogMessageError(loc, eLogMessageType::Critical, formatted);
			DebugOutputIDE(output);
		}

	private:
		static std::string MakeLogMessage(const std::source_location& loc, eLogMessageType type, const std::string& msg);
		static std::string MakeLogMessageError(const std::source_location& loc, eLogMessageType type, const std::string& msg);
		static constexpr const char* LogMessageTypeToString(eLogMessageType type)
		{
			switch (type)
			{
			case eLogMessageType::Message:		return "MESSAGE";
			case eLogMessageType::Warning:		return "WARNING";
			case eLogMessageType::Error:		return "ERROR";
			case eLogMessageType::Exception:	return "EXCEPTION";
			case eLogMessageType::Critical:		return "CRITICAL";
			case eLogMessageType::Fatal:		return "FATAL";
			}

			std::unreachable();
		}
		static constexpr const char* GetPlatformLogLineEnding()
		{
			auto end = "\n";

			// Для Linux-подобных платформ (включая Android) символ '\n' не требуется,
			// поскольку средства просмотра логов (IDE, Logcat и т.п.) сами разделяют записи.
#if defined(__linux__)
			end = "";
#endif
			return end;
		}

		static void DebugOutputIDE(const std::string& output) noexcept;
	};
}
