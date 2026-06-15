#pragma once

#include "header.h"

#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD

#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <memory>
#include <optional>
#include <common/common.h>
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

		/**
		 * @brief Добавляет бродкастер для вывода логов в консоль.
		 * @details Работает на Windows. Автоматически аллоцирует консольное окно,
		 *          если оно отсутствует, и перенаправляет туда форматированный вывод.
		 */
		void AddConsoleBroadcaster();

		void LogMessage(const std::source_location& loc, std::string formatted);
		void LogWarning(const std::source_location& loc, std::string formatted);
		void LogError(const std::source_location& loc, std::string formatted);
		void LogException(const std::source_location& loc, std::string formatted);
		void LogCritical(const std::source_location& loc, std::string formatted);

	private:
		void AddBroadcasterImpl(std::shared_ptr<IBroadcaster> broadcaster);
		void ProcessLog(const std::source_location& loc, eLogMessageType type, std::string formatted);
		void AddToBroadcast(const std::source_location& loc, eLogMessageType type, std::string msg);
		void StartBroadcastThreadIfNeeded();
		void BroadcastThreadLoop();
		void BroadcastLogs(const std::vector<LogEntry>& logs);

		void DebugOutputIDE(const std::source_location& loc, eLogMessageType type, const std::string& formatted) noexcept;
		std::string MakeLogMessage(const std::source_location& loc, eLogMessageType type, const std::string& msg);
		std::string MakeLogMessageError(const std::source_location& loc, eLogMessageType type, const std::string& msg);
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
	 */
	inline Logger g_Logger;
}
#endif