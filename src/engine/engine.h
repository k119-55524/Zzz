#pragma once

#include <mutex>
#include <atomic>
#include <memory>
#include <expected>
#include <string_view>

#include "NativeAppData.h"
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
using namespace zzz::common;
using namespace zzz::logger;

namespace zzz::engine
{
	class Engine
	{
	public:
		Engine() = delete;
		Engine(std::string_view appName, std::shared_ptr<NativeAppData> nativeData = nullptr);
		~Engine();

		static Engine& Get();

		[[nodiscard]] std::expected<void, std::string> Run();

	protected:
		void Shutdown();
		void OnUpdateSystem();

		inline static Engine* s_Instance = nullptr;
		std::mutex stateMutex;
		std::atomic<eInitState> engineState;

		std::unique_ptr<Platform> m_Platform;
		std::unique_ptr<ViewManager> m_ViewManager;
		std::shared_ptr<MainLoopBase> m_MainLoop;

	private:
		void Initialize();
		void OnCloseAllViews();
	};
}
