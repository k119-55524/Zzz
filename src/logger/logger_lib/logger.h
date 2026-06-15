#pragma once

#include "header.h"

namespace zzz::logger
{
	struct LogEntry
	{
		uint64_t timestamp;
		eLogMessageType type;
		std::string text;
		std::string file;
		std::string function;
		uint32_t line;
	};

	class Logger
	{
	public:
		static void Initialize(eLogMessageType filterMask = eLogMessageType::All, bool enableStreaming = true);

		template<typename... Args>
		static void LogMessage(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			ProcessLog(loc, eLogMessageType::Message, std::format(fmt, std::forward<Args>(args)...));
#endif
		}

		template<typename... Args>
		static void LogWarning(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			ProcessLog(loc, eLogMessageType::Warning, std::format(fmt, std::forward<Args>(args)...));
#endif
		}

		template<typename... Args>
		static void LogError(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			ProcessLog(loc, eLogMessageType::Error, std::format(fmt, std::forward<Args>(args)...));
#endif
		}

		template<typename... Args>
		static void LogException(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			ProcessLog(loc, eLogMessageType::Exception, std::format(fmt, std::forward<Args>(args)...));
#endif
		}

		template<typename... Args>
		static void LogCritical(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
		{
			ProcessLog(loc, eLogMessageType::Critical, std::format(fmt, std::forward<Args>(args)...));
		}

	private:
		static void ProcessLog(const std::source_location& loc, eLogMessageType type, std::string formatted);
		static void AddToBroadcast(const std::source_location& loc, eLogMessageType type, std::string msg);
		static void BroadcastThreadLoop();
		static void BroadcastLogs(const std::vector<LogEntry>& logs);

		static void DebugOutputIDE(const std::source_location& loc, eLogMessageType type, const std::string& formatted) noexcept;
		static std::string MakeLogMessage(const std::source_location& loc, eLogMessageType type, const std::string& msg);
		static std::string MakeLogMessageError(const std::source_location& loc, eLogMessageType type, const std::string& msg);
		static constexpr const char* LogMessageTypeToString(eLogMessageType type)
		{
			if (!!(type & eLogMessageType::Message))   return "MESSAGE";
			if (!!(type & eLogMessageType::Warning))   return "WARNING";
			if (!!(type & eLogMessageType::Error))     return "ERROR";
			if (!!(type & eLogMessageType::Exception)) return "EXCEPTION";
			if (!!(type & eLogMessageType::Critical))  return "CRITICAL";
			if (!!(type & eLogMessageType::Fatal))     return "FATAL";
			return "UNKNOWN";
		}
		static constexpr const char* GetPlatformLogLineEnding()
		{
#if Z_LINUX || Z_ANDROID
			return "";
#else
			return "\n";
#endif
		}
		
		static std::atomic<eLogMessageType> AllowedOutputTypesMask;
	};
}