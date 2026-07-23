#pragma once

#include <mutex>
#include <atomic>
#include <memory>
#include <vector>
#include <expected>
#include <string_view>

#include "public/core/EngineTime.h"
#include "NativeAppData.h"
#include <logger/logger.h>

namespace zzz
{
	enum class eInitState : zU8;
}

namespace zzz::script
{
	class GameScript;
}

namespace zzz::engine
{
	class Platform;
	class ViewManager;
	class MainLoopBase;
	class ProjectEventBus;
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

		[[nodiscard]] virtual std::expected<void, std::string> Run();

	protected:
		void Shutdown();

		virtual void StartGame(const std::vector<std::string>& globalScripts);
		virtual void OnRegisterScripts();
		virtual void OnUpdateSystem();

		void LoadGlobalScripts(const std::vector<std::string>& globalScripts);

		std::mutex stateMutex;
		std::atomic<eInitState> engineState;
		std::vector<std::shared_ptr<zzz::script::GameScript>> m_Scripts;

	protected:
		std::unique_ptr<Platform> m_Platform;
		std::unique_ptr<ViewManager> m_ViewManager;
		std::shared_ptr<MainLoopBase> m_MainLoop;
		std::shared_ptr<ProjectEventBus> m_EventBus;
		std::shared_ptr<Time> m_Time;

	private:
		void Initialize();
		void OnCloseAllViews() const;
	};
}
