#pragma once

#include "header.h"

#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD

#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <memory>
#include <common/common.h>
#include <condition_variable>

#include "log_entry.h"

using namespace zzz::common;

namespace zzz::logger
{
	class IBroadcaster;

	struct LogCallbackEntry
	{
		uint64_t timestamp;
		int type;
		const char* text;
		const char* file;
		const char* function;
		uint32_t line;
	};

	typedef void (__stdcall *LogCallback)(const LogCallbackEntry& entry);

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

		/**
		 * @brief Добавляет сетевой бродкастер (TCP).
		 */
		void AddNetworkBroadcaster(std::string_view address, uint16_t port);

		/**
		 * @brief Добавляет колбэк-бродкастер для перенаправления логов во внешнюю функцию.
		 */
		void AddCallbackBroadcaster(LogCallback callback);

		/**
		 * @brief Останавливает и джойнит фоновый поток рассылки логов (если он запущен).
		 * @details Нужно вызывать явно до FreeLibrary() модуля, который содержит этот
		 *          экземпляр Logger (например, scripts.dll) - ждать поток внутри
		 *          DllMain(DLL_PROCESS_DETACH), как это делает деструктор, небезопасно:
		 *          и основной поток, и завершающийся фоновый поток претендуют на loader lock,
		 *          что даёт гарантированный deadlock. Вызов отсюда, из обычного кода вне
		 *          DllMain, безопасен, а деструктор после этого просто не найдёт что джойнить.
		 */
		void StopBroadcastThread();

		void LogMessage(const std::source_location& loc, std::string formatted);
		void LogWarning(const std::source_location& loc, std::string formatted);
		void LogError(const std::source_location& loc, std::string formatted);
		void LogException(const std::source_location& loc, std::string formatted);
		void LogCritical(const std::source_location& loc, std::string formatted);
		void LogFatal(const std::source_location& loc, std::string formatted);

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

	inline Logger g_Logger;
}
#endif