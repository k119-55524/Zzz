
#include "private/IBroadcaster.h"
#include "private/ConsoleBroadcaster.h"
#include "private/NetworkBroadcaster.h"
#include "private/CallbackBroadcaster.h"
#include "core/enums/eLogMessageType.h"
#include "core/utils/MemoryUtils.h"

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
	if (!m_BroadcastThread.joinable())
		return;

	m_BroadcastThread.request_stop();
	m_BroadcastCV.notify_all();
	m_BroadcastThread.join();
}

void Logger::SetLogFilterMask(eLogMessageType filterMask)
{
	g_Logger.m_FilterMask.store(filterMask);
}

#pragma region Категории
bool Logger::IsCategoryEnabled(const LogCategory& category) const
{
	if (m_BypassAllFilters.load(std::memory_order_relaxed))
		return true;

	if (category.IsGuaranteed())
		return true;

	const bool groupEnabled = category.IsEngine()
		? m_EngineGroupEnabled.load(std::memory_order_relaxed)
		: m_UserGroupEnabled.load(std::memory_order_relaxed);
	if (!groupEnabled)
		return false;

	std::shared_lock lock(m_DisabledCategoriesMutex);
	return !m_DisabledCategories.contains(category.name);
}

void Logger::SetCategoryEnabled(std::string_view categoryName, bool enabled)
{
	std::unique_lock lock(m_DisabledCategoriesMutex);
	if (enabled)
		m_DisabledCategories.erase(std::string(categoryName));
	else
		m_DisabledCategories.insert(std::string(categoryName));
}

void Logger::SetGroupEnabled(eLogCategoryGroup group, bool enabled) noexcept
{
	if (group == eLogCategoryGroup::Engine)
		m_EngineGroupEnabled.store(enabled, std::memory_order_relaxed);
	else
		m_UserGroupEnabled.store(enabled, std::memory_order_relaxed);
}

void Logger::SetBypassAllFilters(bool bypass) noexcept
{
	m_BypassAllFilters.store(bypass, std::memory_order_relaxed);
}
#pragma endregion

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
void Logger::LogMessage(const std::source_location& loc, const LogCategory& category, std::string formatted)
{
#if Z_ADD_LOGGER
	ProcessLog(loc, eLogMessageType::Message, category, std::move(formatted));
#else
	(void)loc;
	(void)category;
	(void)formatted;
#endif
}

void Logger::LogWarning(const std::source_location& loc, const LogCategory& category, std::string formatted)
{
	ProcessLog(loc, eLogMessageType::Warning, category, std::move(formatted));
}

void Logger::LogError(const std::source_location& loc, const LogCategory& category, std::string formatted)
{
	ProcessLog(loc, eLogMessageType::Error, category, std::move(formatted));
}

void Logger::LogException(const std::source_location& loc, const LogCategory& category, std::string formatted)
{
	ProcessLog(loc, eLogMessageType::Exception, category, std::move(formatted));
}

void Logger::LogCritical(const std::source_location& loc, const LogCategory& category, std::string formatted)
{
	ProcessLog(loc, eLogMessageType::Critical, category, std::move(formatted));
}

void Logger::LogFatal(const std::source_location& loc, const LogCategory& category, std::string formatted)
{
	ProcessLog(loc, eLogMessageType::Fatal, category, std::move(formatted));
	std::terminate();
}
#pragma endregion

void Logger::ProcessLog(const std::source_location& loc, eLogMessageType type, const LogCategory& category, std::string formatted)
{
	auto mask = m_FilterMask.load();
	if (!(mask & type))
		return;

	DebugOutputIDE(loc, type, category, formatted);

	bool hasListeners = false;
	{
		std::lock_guard lock(m_ListenersMutex);
		hasListeners = !m_Listeners.empty();
	}

	if (hasListeners)
	{
		AddToBroadcast(loc, type, category, std::move(formatted));
	}
}

void Logger::AddToBroadcast(const std::source_location& loc, eLogMessageType type, const LogCategory& category, std::string msg)
{
	if (!m_BroadcastThread.joinable() || m_BroadcastThread.get_stop_token().stop_requested())
		return;

	auto now = std::chrono::system_clock::now();
	auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

	m_LogBuffer.Emplace(
		timestamp,
		type,
		&category,
		std::move(msg),
		loc.file_name() ? loc.file_name() : "",
		loc.function_name() ? loc.function_name() : "",
		loc.line()
	);

	m_BroadcastCV.notify_one();
}

void Logger::DebugOutputIDE(const std::source_location& loc, eLogMessageType type, const LogCategory& category, const std::string& formatted) noexcept
{
#if Z_IDE_OUT_LOGS
	std::string output;
	if (!!(type & (eLogMessageType::Message | eLogMessageType::Warning)))
	{
		output = MakeLogMessage(loc, type, category, formatted);
	}
	else
	{
		output = MakeLogMessageError(loc, type, category, formatted);
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

namespace
{
	// Пустой source_location (см. GAPIDebugLogger.cpp - Report() передаёт std::source_location{} напрямую,
	// т.к. call site там всегда одна и та же строка debug-колбэка GAPI, показывать нечего) значит "call site не показывать".
	bool HasCallSite(const std::source_location& loc)
	{
		return loc.file_name() && *loc.file_name() != '\0';
	}
}

std::string Logger::MakeLogMessage(const std::source_location& loc, eLogMessageType type, const LogCategory& category, const std::string& msg)
{
	if (!!(type & eLogMessageType::Message) || !HasCallSite(loc))
		return std::format(
			">>>>> [{}] [{}] {}{}",
			ToString(type),
			category.name,
			msg,
			GetPlatformLogLineEnding());
	else
		return std::format(
			">>>>> [{}] [{}] {} -> line: {}, file: {}{}",
			ToString(type),
			category.name,
			msg,
			loc.line(),
			loc.file_name(),
			GetPlatformLogLineEnding());
}

std::string Logger::MakeLogMessageError(const std::source_location& loc, eLogMessageType type, const LogCategory& category, const std::string& msg)
{
	if (!HasCallSite(loc))
		return std::format(
			">>>>> [{}] [{}] {}{}",
			ToString(type),
			category.name,
			msg,
			GetPlatformLogLineEnding());

	return std::format(
		">>>>> [{}] [{}] {} -> [{}]. line: {}, file: {}{}",
		ToString(type),
		category.name,
		msg,
		loc.function_name(),
		loc.line(),
		loc.file_name(),
		GetPlatformLogLineEnding());
}

void Logger::BroadcastThreadLoop(std::stop_token stopToken)
{
	for (;;)
	{
		auto& readBuffer = m_LogBuffer.SwapAndGetReadBuffer();

		if (!readBuffer.empty())
		{
			BroadcastLogs(readBuffer);
			if (stopToken.stop_requested())
				break;
			continue;
		}

		if (stopToken.stop_requested())
			break;

		std::unique_lock<std::mutex> lock(m_BroadcastMutex);
		m_BroadcastCV.wait(lock, stopToken, [this]() {
			return !m_LogBuffer.IsEmpty();
		});
	}
}

void Logger::StartBroadcastThreadIfNeeded()
{
	std::lock_guard lock(m_BroadcastMutex);
	if (m_BroadcastThread.joinable())
		return;
	m_BroadcastThread = std::jthread([this](std::stop_token st) {
		BroadcastThreadLoop(std::move(st));
	});
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
