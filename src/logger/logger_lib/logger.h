#pragma once

#include "header.h"

#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD

#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <memory>
#include <optional>
#include <common/templates/double_buffered_vector.h>
#include <condition_variable>

namespace zzz::logger
{
	struct LogEntry
	{
		uint64_t timestamp;
		zzz::common::eLogMessageType type;
		std::string text;
		std::string file;
		std::string function;
		uint32_t line;
	};

	class Logger
	{
	public:
		Logger(bool enableStreaming, zzz::common::eLogMessageType filterMask);
		~Logger();

		static void Initialize(bool enableStreaming = true, zzz::common::eLogMessageType filterMask = zzz::common::eLogMessageType::All);

		void LogMessage(const std::source_location& loc, std::string formatted);
		void LogWarning(const std::source_location& loc, std::string formatted);
		void LogError(const std::source_location& loc, std::string formatted);
		void LogException(const std::source_location& loc, std::string formatted);
		void LogCritical(const std::source_location& loc, std::string formatted);

	private:
		void ProcessLog(const std::source_location& loc, zzz::common::eLogMessageType type, std::string formatted);
		void AddToBroadcast(const std::source_location& loc, zzz::common::eLogMessageType type, std::string msg);
		void BroadcastThreadLoop();
		void BroadcastLogs(const std::vector<LogEntry>& logs);

		void DebugOutputIDE(const std::source_location& loc, zzz::common::eLogMessageType type, const std::string& formatted) noexcept;
		std::string MakeLogMessage(const std::source_location& loc, zzz::common::eLogMessageType type, const std::string& msg);
		std::string MakeLogMessageError(const std::source_location& loc, zzz::common::eLogMessageType type, const std::string& msg);
		constexpr const char* LogMessageTypeToString(zzz::common::eLogMessageType type)
		{
			if (!!(type & zzz::common::eLogMessageType::Message))   return "MESSAGE";
			if (!!(type & zzz::common::eLogMessageType::Warning))   return "WARNING";
			if (!!(type & zzz::common::eLogMessageType::Error))     return "ERROR";
			if (!!(type & zzz::common::eLogMessageType::Exception)) return "EXCEPTION";
			if (!!(type & zzz::common::eLogMessageType::Critical))  return "CRITICAL";
			if (!!(type & zzz::common::eLogMessageType::Fatal))     return "FATAL";
			return "UNKNOWN";
		}
		constexpr const char* GetPlatformLogLineEnding()
		{
#if Z_LINUX || Z_ANDROID
			return "";
#else
			return "\n";
#endif
		}
		
		std::atomic<zzz::common::eLogMessageType> m_AllowedOutputTypesMask;
		
		zzz::common::DoubleBufferedVector<LogEntry> m_LogBuffer;
		std::thread m_BroadcastThread;
		std::condition_variable m_BroadcastCV;
		std::mutex m_BroadcastMutex;
		std::atomic<bool> m_BroadcastThreadRunning{false};
	};

	inline std::optional<Logger> g_Logger;
}

#endif