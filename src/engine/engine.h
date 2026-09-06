#pragma once

#include "engine/gapi/GAPI.h"
#include "core/enums/eInitState.h"
#include "engine/view/ViewManager.h"
#include "engine/platforms/Platform.h"
#include "engine/scene/SceneManager.h"
#include "engine/package/PackageManager.h"
#include "core/io/package/DataAssetsManager.h"
#include "engine/resources/ResourceManager.h"
#include "engine/resources/ResourceGarbageCollector.h"
#include "engine/package/UserSettingsManager.h"
#include "engine/platforms/mainloop/MainLoop.h"

using namespace zzz::core;

namespace zzz::engine
{
	class Engine
	{
	public:
		Engine(std::shared_ptr<NativeAppData> nativeData = nullptr);
		~Engine();

		[[nodiscard]] virtual std::expected<void, std::string> Run();
		[[nodiscard]] std::shared_ptr<DataAssetsManager> GetDataAssetsManager() const noexcept { return m_DataAssetsManager; }
		[[nodiscard]] std::shared_ptr<ResourceManager> GetResourceManager() const noexcept { return m_ResourceManager; }
		[[nodiscard]] ResourceGarbageCollector* GetResourceGC() const noexcept { return m_ResourceGC.get(); }

	protected:
		void Shutdown() noexcept;

		virtual void StartGame() {};
		virtual void StopGame();
		virtual void RegisterScripts();
		virtual void OnUpdateSystem();

		void LoadGlobalScripts();

		std::mutex stateMutex;
		std::atomic<eInitState> engineState;
		std::vector<std::shared_ptr<GameScript>> m_Scripts;

		std::shared_ptr<ScriptStorage> m_ScriptStorage;
		std::unique_ptr<ScriptRegistry> m_ScriptRegistry;
		std::shared_ptr<ScriptFactory> m_ScriptFactory;

		std::shared_ptr<FileSystem> m_FileSystem;
		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<DataAssetsManager> m_DataAssetsManager;
		std::shared_ptr<ResourceManager> m_ResourceManager;
		std::unique_ptr<ResourceGarbageCollector> m_ResourceGC;
		std::shared_ptr<UserSettingsManager> m_UserSettingsManager;
		std::unique_ptr<Platform> m_Platform;
		std::shared_ptr<GAPI> m_GAPI;
		std::shared_ptr<SceneManager> m_SceneManager;
		std::unique_ptr<ViewManager> m_ViewManager;
		std::shared_ptr<MainLoopBase> m_MainLoop;
		std::shared_ptr<ProjectEventBus> m_EventBus;
		std::shared_ptr<Time> m_Time;

	private:
		void OnAppClosed() const;
	};
}
