#pragma once

#include <mutex>
#include <atomic>
#include <string>
#include <vector>
#include <memory>
#include <expected>

#include "engine/gapi/GAPI.h"
#include "core/enums/eInitState.h"
#include "core/utils/NativeAppData.h"
#include "core/io/storage/FileSystem.h"

namespace zzz::core
{
	class Time;
	class GameScript;
	class ScriptStorage;
	class ScriptRegistry;
	class ScriptFactory;
	class ProjectEventBus;
	class DataAssetsManager;
}

namespace zzz::engine
{
	class Platform;
	class ViewManager;
	class SceneManager;
	class MainLoopBase;
	class PackageManager;
	class TaskDispatcher;
	class CpuResourceManager;
	class GpuResourceManager;
	class UserSettingsManager;

	class Engine
	{
	public:
		Engine(std::shared_ptr<zzz::core::NativeAppData> nativeData = nullptr);
		virtual ~Engine();

		[[nodiscard]] virtual std::expected<void, std::string> Run();

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

		std::shared_ptr<Time> m_Time;
		std::unique_ptr<TaskDispatcher> m_TaskDispatcher;
		std::unique_ptr<Platform> m_Platform;
		std::shared_ptr<GAPI> m_GAPI;
		std::shared_ptr<FileSystem> m_FileSystem;
		std::shared_ptr<MainLoopBase> m_MainLoop;
		std::shared_ptr<ProjectEventBus> m_EventBus;

		std::shared_ptr<ScriptStorage> m_ScriptStorage;
		std::unique_ptr<ScriptRegistry> m_ScriptRegistry;
		std::shared_ptr<ScriptFactory> m_ScriptFactory;

		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<DataAssetsManager> m_DataAssetsManager;
		std::shared_ptr<CpuResourceManager> m_CpuResourceManager;
		std::shared_ptr<GpuResourceManager> m_GpuResourceManager;
		std::shared_ptr<UserSettingsManager> m_UserSettingsManager;
		std::shared_ptr<SceneManager> m_SceneManager;
		std::unique_ptr<ViewManager> m_ViewManager;

	private:
		void OnAppClosed() const;
	};
}
