#include "logger.h"
#include <common/common.h>
using namespace zzz::common;
using namespace zzz::logger;

std::atomic<eLogMessageType> Logger::AllowedOutputTypesMask = eLogMessageType::All;

namespace
{
	DoubleBufferedVector<LogEntry> g_LogBuffer;
	std::thread g_BroadcastThread;
	std::condition_variable g_BroadcastCV;
	std::mutex g_BroadcastMutex;
	std::atomic<bool> g_BroadcastThreadRunning = false;

	struct LoggerCleaner
	{
		~LoggerCleaner()
		{
			// Гарантируем остановку потока при выходе из приложения
			bool needJoin = false;

			{
				std::lock_guard<std::mutex> lock(g_BroadcastMutex);

				if (!g_BroadcastThreadRunning.load())
					return;

				g_BroadcastThreadRunning.store(false);
				needJoin = g_BroadcastThread.joinable();
			}

			g_BroadcastCV.notify_one();

			if (needJoin)
			{
				g_BroadcastThread.join();
			}
		}
	} g_LoggerCleaner;
}

void Logger::Initialize(eLogMessageType filterMask, bool enableStreaming)
{
	static std::atomic<bool> isInit{false};
	if (isInit.exchange(true))
		THROW_RUNTIME("Logger has already been initialized.");

	AllowedOutputTypesMask.store(filterMask);

	if (enableStreaming)
	{
		std::lock_guard<std::mutex> lock(g_BroadcastMutex);
		g_BroadcastThreadRunning.store(true);
		g_BroadcastThread = std::thread(&Logger::BroadcastThreadLoop);
	}
}

void Logger::ProcessLog(const std::source_location& loc, eLogMessageType type, std::string formatted)
{
	auto mask = AllowedOutputTypesMask.load();
	if (!(mask & type))
		return;

	DebugOutputIDE(loc, type, formatted);
	AddToBroadcast(loc, type, std::move(formatted));
}

void Logger::AddToBroadcast(const std::source_location& loc, eLogMessageType type, std::string msg)
{
	if (!g_BroadcastThreadRunning.load())
		return;

	auto now = std::chrono::system_clock::now();
	auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

	g_LogBuffer.Emplace(
		timestamp,
		type,
		std::move(msg),
		loc.file_name() ? loc.file_name() : "",
		loc.function_name() ? loc.function_name() : "",
		loc.line()
	);

	g_BroadcastCV.notify_one();
}

void Logger::BroadcastThreadLoop()
{
	for (;;)
	{
		auto& readBuffer = g_LogBuffer.SwapAndGetReadBuffer();

		if (!readBuffer.empty())
		{
			BroadcastLogs(readBuffer);
			readBuffer.clear(); // Очищаем буфер после отправки
			continue;
		}

		if (!g_BroadcastThreadRunning.load())
			break;

		std::unique_lock<std::mutex> lock(g_BroadcastMutex);
		g_BroadcastCV.wait(lock, []() {
			return !g_BroadcastThreadRunning.load() || !g_LogBuffer.IsEmpty();
		});
	}
}

void Logger::BroadcastLogs(const std::vector<LogEntry>& logs)
{
	// TODO: Сетевая отправка
	// Заглушка, чтобы компилятор не ругался на неиспользуемый параметр
	(void)logs;
}

void Logger::DebugOutputIDE(const std::source_location& loc, eLogMessageType type, const std::string& formatted) noexcept
{
#if Z_IDE_OUT_LOGS
	std::string output;
	if (!!(type & (eLogMessageType::Message | eLogMessageType::Warning)))
	{
		output = MakeLogMessage(loc, type, formatted);
	}
	else
	{
		output = MakeLogMessageError(loc, type, formatted);
	}

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

std::string Logger::MakeLogMessage(const std::source_location& loc, eLogMessageType type, const std::string& msg)
{
	if (!!(type & eLogMessageType::Message))
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