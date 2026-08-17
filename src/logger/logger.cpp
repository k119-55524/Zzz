
#include "private/IBroadcaster.h"
#include "private/ConsoleBroadcaster.h"
#include "private/NetworkBroadcaster.h"
#include "private/CallbackBroadcaster.h"

#include "logger.h"

using namespace zzz::core;
using namespace zzz::logger;

Logger::Logger()
{
	m_FilterMask.store(eLogMessageType::All);
}

Logger::~Logger()
{
	StopBroadcastThread();
}

void Logger::StopBroadcastThread()
{
	bool needJoin = false;

	{
		std::lock_guard<std::mutex> lock(m_BroadcastMutex);

		if (!m_BroadcastThreadRunning.load())
			return;

		m_BroadcastThreadRunning.store(false);
		needJoin = m_BroadcastThread.joinable();
	}

	m_BroadcastCV.notify_one();

	if (needJoin)
	{
		m_BroadcastThread.join();
	}
}

void Logger::SetLogFilterMask(eLogMessageType filterMask)
{
	g_Logger.m_FilterMask.store(filterMask);
}

void Logger::SetMaxNetworkLogQueueSize(zU32 newSize)
{
	if (newSize == 0)
		return;

	std::vector<std::shared_ptr<IBroadcaster>> listeners;
	{
		std::lock_guard lock(m_ListenersMutex);
		listeners = m_Listeners;
	}

	for (const auto& listener : listeners)
	{
		listener->SetMaxQueueSize(newSize);
	}
}

#pragma region LogXXX messages
void Logger::LogMessage(const std::source_location& loc, std::string formatted)
{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
	ProcessLog(loc, eLogMessageType::Message, std::move(formatted));
#else
	(void)loc;
	(void)formatted;
#endif
}

void Logger::LogWarning(const std::source_location& loc, std::string formatted)
{
	ProcessLog(loc, eLogMessageType::Warning, std::move(formatted));
}

void Logger::LogError(const std::source_location& loc, std::string formatted)
{
	ProcessLog(loc, eLogMessageType::Error, std::move(formatted));
}

void Logger::LogException(const std::source_location& loc, std::string formatted)
{
	ProcessLog(loc, eLogMessageType::Exception, std::move(formatted));
}

void Logger::LogCritical(const std::source_location& loc, std::string formatted)
{
	ProcessLog(loc, eLogMessageType::Critical, std::move(formatted));
}

void Logger::LogFatal(const std::source_location& loc, std::string formatted)
{
	ProcessLog(loc, eLogMessageType::Fatal, std::move(formatted));
	std::terminate();
}
#pragma endregion

void Logger::ProcessLog(const std::source_location& loc, eLogMessageType type, std::string formatted)
{
	auto mask = m_FilterMask.load();
	if (!(mask & type))
		return;

	DebugOutputIDE(loc, type, formatted);

	bool hasListeners = false;
	{
		std::lock_guard lock(m_ListenersMutex);
		hasListeners = !m_Listeners.empty();
	}

	if (hasListeners)
	{
		AddToBroadcast(loc, type, std::move(formatted));
	}
}

void Logger::AddToBroadcast(const std::source_location& loc, eLogMessageType type, std::string msg)
{
	if (!m_BroadcastThreadRunning.load())
		return;

	auto now = std::chrono::system_clock::now();
	auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

	m_LogBuffer.Emplace(
		timestamp,
		type,
		std::move(msg),
		loc.file_name() ? loc.file_name() : "",
		loc.function_name() ? loc.function_name() : "",
		loc.line()
	);

	m_BroadcastCV.notify_one();
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
	{
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, output.c_str(), (int)output.size(), NULL, 0);
		std::wstring wstrTo(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, output.c_str(), (int)output.size(), &wstrTo[0], size_needed);
		OutputDebugStringW(wstrTo.c_str());
	}
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
			EnumToString::ToString(type),
			msg,
			GetPlatformLogLineEnding());
	else
		return std::format(
			">>>>> [{}] {} -> line: {}, file: {}{}",
			EnumToString::ToString(type),
			msg,
			loc.line(),
			loc.file_name(),
			GetPlatformLogLineEnding());
}

std::string Logger::MakeLogMessageError(const std::source_location& loc, eLogMessageType type, const std::string& msg)
{
	return std::format(
		">>>>> [{}] {} -> [{}]. line: {}, file: {}{}",
		EnumToString::ToString(type),
		msg,
		loc.function_name(),
		loc.line(),
		loc.file_name(),
		GetPlatformLogLineEnding());
}

void Logger::BroadcastThreadLoop()
{
	for (;;)
	{
		auto& readBuffer = m_LogBuffer.SwapAndGetReadBuffer();

		if (!readBuffer.empty())
		{
			BroadcastLogs(readBuffer);
			if (!m_BroadcastThreadRunning.load())
				break;
			continue;
		}

		if (!m_BroadcastThreadRunning.load())
			break;

		std::unique_lock<std::mutex> lock(m_BroadcastMutex);
		m_BroadcastCV.wait(lock, [this]() {
			return !m_BroadcastThreadRunning.load() || !m_LogBuffer.IsEmpty();
		});
	}
}

void Logger::StartBroadcastThreadIfNeeded()
{
	std::lock_guard lock(m_BroadcastMutex);
	if (m_BroadcastThreadRunning.load())
		return;
	m_BroadcastThreadRunning.store(true);
	m_BroadcastThread = std::thread(&Logger::BroadcastThreadLoop, this);
}

void Logger::BroadcastLogs(const std::vector<LogEntry>& logs)
{
	std::vector<std::shared_ptr<IBroadcaster>> listeners;
	{
		std::lock_guard lock(m_ListenersMutex);
		listeners = m_Listeners;
	}

	if (listeners.empty() || logs.empty())
		return;

	std::span<const LogEntry> batchSpan(logs.data(), logs.size());
	for (const auto& listener : listeners)
	{
		listener->PushLogsBatch(batchSpan);
	}
}

#pragma region Add broadcasters
void Logger::AddBroadcasterImpl(std::shared_ptr<IBroadcaster> broadcaster)
{
	{
		std::lock_guard lock(m_ListenersMutex);
		m_Listeners.push_back(std::move(broadcaster));
	}

	StartBroadcastThreadIfNeeded();
}

void Logger::AddConsoleBroadcaster()
{
#if Z_WINDOWS
	AddBroadcasterImpl(safe_make_shared<ConsoleBroadcaster>());
#else
	DOutWarning("Logger::AddConsoleBroadcaster(). ConsoleBroadcaster в данный момент поддерживается только на Windows.");
#endif
}

void Logger::AddNetworkBroadcaster(std::string_view address, uint16_t port, zU32 maxQueueSize)
{
	AddBroadcasterImpl(safe_make_shared<NetworkBroadcaster>(address, port, maxQueueSize));
}

void Logger::AddCallbackBroadcaster(LogCallback callback)
{
	AddBroadcasterImpl(safe_make_shared<CallbackBroadcaster>(callback));
}
#pragma endregion
