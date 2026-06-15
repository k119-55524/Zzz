#pragma once

#include <mutex>
#include <atomic>
#include <memory>
#include <expected>
#include <string_view>

#include "NativeAppData.h"
#include <common/common.h>
#include <logger/logger_lib/logger.h>

namespace zzz
{
	enum class eInitState : zU8;
}

namespace zzz::engine
{
	class Platform;
	class ViewManager;
	class MainLoopBase;
}

using namespace zzz;
using namespace zzz::logger;

namespace zzz::engine
{
	class Engine final
	{
	public:
		Engine() = delete;
		Engine(std::string_view appName, std::shared_ptr<NativeAppData> nativeData = nullptr);
		~Engine();

		static Engine& Get();

		[[nodiscard]] std::expected<void, std::string> Initialize();
		[[nodiscard]] std::expected<void, std::string> Run();

		inline static void CustomInitLogger(bool enableStreaming, eLogMessageType filterMask = eLogMessageType::All) { zzz::logger::Logger::Initialize(filterMask, enableStreaming); }

	private:
		void Shutdown();
		void OnCloseAllViews();
		void OnUpdateSystem();

		inline static Engine* s_Instance = nullptr;

		std::mutex stateMutex;
		std::atomic<eInitState> engineState;

		std::shared_ptr<Platform> m_Platform;
		std::unique_ptr<ViewManager> m_ViewManager;
		std::shared_ptr<MainLoopBase> m_MainLoop;
	};
}