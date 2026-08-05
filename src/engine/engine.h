#pragma once

#include <engine/EngineIncludes.h>
#include <engine/utils/Fwd.h>

using namespace zzz::core;

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

		virtual void StartGame() {};
		virtual void StopGame() {};
		virtual void RegisterScripts();
		virtual void OnUpdateSystem();

		void LoadGlobalScripts();

		std::mutex stateMutex;
		std::atomic<eInitState> engineState;
		std::vector<std::shared_ptr<GameScript>> m_Scripts;

		std::shared_ptr<Path> m_Path;
		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<UserSettingsManager> m_UserSettingsManager;
		std::unique_ptr<Platform> m_Platform;
		std::unique_ptr<ViewManager> m_ViewManager;
		std::shared_ptr<MainLoopBase> m_MainLoop;
		std::shared_ptr<ProjectEventBus> m_EventBus;
		std::shared_ptr<Time> m_Time;

	private:
		void OnCloseAllViews() const;
	};
}
