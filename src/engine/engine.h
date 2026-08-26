#pragma once

#include "engine/gapi/GAPI.h"
#include "engine/view/ViewManager.h"
#include "engine/platforms/Platform.h"
#include "engine/package/PackageManager.h"
#include "engine/package/UserSettingsManager.h"
#include "engine/platforms/mainloop/MainLoop.h"

using namespace zzz::core;

namespace zzz::engine
{
	class Engine
	{
	public:
		Engine() = delete;

		/**
		 * @brief Инициализирует подсистемы движка (сетевой логер, пути, манифест, GAPI, скрипты и окна).
		 * @param appName Имя приложения.
		 * @param nativeData Нативные данные платформы.
		 */
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

		std::shared_ptr<ScriptStorage> m_ScriptStorage;
		std::unique_ptr<ScriptRegistry> m_ScriptRegistry;
		std::shared_ptr<ScriptFactory> m_ScriptFactory;

		std::shared_ptr<Path> m_Path;
		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<UserSettingsManager> m_UserSettingsManager;
		std::unique_ptr<Platform> m_Platform;
		std::shared_ptr<GAPI> m_GAPI;
		std::unique_ptr<ViewManager> m_ViewManager;
		std::shared_ptr<MainLoopBase> m_MainLoop;
		std::shared_ptr<ProjectEventBus> m_EventBus;
		std::shared_ptr<Time> m_Time;

	private:
		void OnAppClosed() const;
	};
}
