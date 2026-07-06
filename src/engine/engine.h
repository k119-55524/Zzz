#pragma once

#include <mutex>
#include <atomic>
#include <memory>
#include <expected>
#include <string_view>
#include <vector>

#include "NativeAppData.h"
#include <logger/logger.h>

namespace zzz
{
	enum class eInitState : zU8;
}

namespace zzz::script
{
	class Game;
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

		[[nodiscard]] virtual std::expected<void, std::string> Run();

	protected:
		virtual void OnRegisterScripts();
		void Shutdown();
		void OnUpdateSystem();

		void StartGame(const std::vector<std::string>& globalScripts);
		void StopGame();
		void PauseGame(bool isPaused);

		std::mutex stateMutex;
		std::atomic<eInitState> engineState;

		bool m_IsTimePaused = false;
		std::vector<std::shared_ptr<zzz::script::Game>> m_GlobalGames;

		std::unique_ptr<Platform> m_Platform;
		std::unique_ptr<ViewManager> m_ViewManager;
		std::shared_ptr<MainLoopBase> m_MainLoop;

	private:
		void Initialize();
		void OnCloseAllViews() const;
	};
}
