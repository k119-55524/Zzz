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

#include "log_entry.h"
#include "private/IBroadcaster.h"

using namespace zzz::common;

namespace zzz::logger
{
	/**
	 * @brief Централизованная система логирования с поддержкой асинхронной рассылки.
	 */
	class Logger
	{
	public:
		Logger();
		~Logger();

		/**
		 * @brief Устанавливает маску типов логов, проходящих через систему.
		 * @note Создаёт глобальный экземпляр логера при первом вызове. Потокобезопасно.
		 * @param filterMask Маска фильтрации для вывода сообщений.
		 */
		static void SetLogFilterMask(eLogMessageType filterMask);

		void LogMessage(const std::source_location& loc, std::string formatted);
		void LogWarning(const std::source_location& loc, std::string formatted);
		void LogError(const std::source_location& loc, std::string formatted);
		void LogException(const std::source_location& loc, std::string formatted);
		void LogCritical(const std::source_location& loc, std::string formatted);

		/**
		 * @brief Добавляет бродкастер типа T, конструируя его с переданными аргументами.
		 * При первом вызове автоматически запускает бродкаст-поток. Потокобезопасно.
		 * @tparam T Конкретный тип бродкастера, должен реализовывать IBroadcaster.
		 * @param args Аргументы конструктора T.
		 */
		template<typename T, typename... Args>
		void AddBroadcaster(Args&&... args)
		{
			auto broadcaster = std::make_shared<T>(std::forward<Args>(args)...);
			{
				std::lock_guard lock(m_ListenersMutex);
				m_Listeners.push_back(std::move(broadcaster));
			}
			StartBroadcastThreadIfNeeded();
		}

	private:
		void ProcessLog(const std::source_location& loc, eLogMessageType type, std::string formatted);
		void AddToBroadcast(const std::source_location& loc, eLogMessageType type, std::string msg);
		void StartBroadcastThreadIfNeeded();
		void BroadcastThreadLoop();
		void BroadcastLogs(const std::vector<LogEntry>& logs);

		void DebugOutputIDE(const std::source_location& loc, eLogMessageType type, const std::string& formatted) noexcept;
		std::string MakeLogMessage(const std::source_location& loc, eLogMessageType type, const std::string& msg);
		std::string MakeLogMessageError(const std::source_location& loc, eLogMessageType type, const std::string& msg);
		constexpr const char* LogMessageTypeToString(eLogMessageType type)
		{
			if (!!(type & eLogMessageType::Message))   return "MESSAGE";
			if (!!(type & eLogMessageType::Warning))   return "WARNING";
			if (!!(type & eLogMessageType::Error))     return "ERROR";
			if (!!(type & eLogMessageType::Exception)) return "EXCEPTION";
			if (!!(type & eLogMessageType::Critical))  return "CRITICAL";
			if (!!(type & eLogMessageType::Fatal))     return "FATAL";
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

		std::atomic<eLogMessageType> m_FilterMask;

		std::vector<std::shared_ptr<IBroadcaster>> m_Listeners;
		std::mutex m_ListenersMutex;

		DoubleBufferedVector<LogEntry> m_LogBuffer;
		std::thread m_BroadcastThread;
		std::condition_variable m_BroadcastCV;
		std::mutex m_BroadcastMutex;
		std::atomic<bool> m_BroadcastThreadRunning{false};
	};

	/**
	 * @brief Глобальный экземпляр логгера.
	 * Инициализируется один раз вызовом Logger::Initialize.
	 */
	inline std::optional<Logger> g_Logger;
}
#endif