#pragma once

#include <mutex>
#include <memory>
#include <string>
#include <expected>
#include <functional>
#include <unordered_map>

#include "core/events/OneShotEvent.h"
#include "engine/tasks/TaskDispatcher.h"
#include "core/templates/CallbackQueue.h"
#include "core/scene/SceneTransitionParams.h"

using namespace zzz::core;
using namespace zzz::templates;

namespace zzz::engine
{
	class PackageManager;
	class Scene;
	class CpuResourceManager;
	class GpuResourceManager;

	class SceneManager final
	{
		Z_NO_COPY_MOVE(SceneManager);

	public:
		SceneManager() = delete;
		SceneManager(
			TaskDispatcher& taskDispatcher,
			std::shared_ptr<PackageManager> packageManager,
			std::shared_ptr<CpuResourceManager> cpuResourceManager,
			std::shared_ptr<GpuResourceManager> gpuResourceManager,
			std::shared_ptr<ScriptFactory> scriptFactory);
		~SceneManager() = default;

		using SceneLoadResult = std::expected<std::shared_ptr<Scene>, std::string>;
		using SceneLoadCallback = std::function<void(SceneLoadResult)>;

		void LoadSceneAsync(Guid sceneGuid, SceneLoadCallback onComplete, eTaskPriority priority = eTaskPriority::Normal);
		void LoadSceneAsync(std::string sceneName, SceneLoadCallback onComplete, eTaskPriority priority = eTaskPriority::Normal);

		void Update(const Time& time);

	private:
		TaskDispatcher& m_TaskDispatcher;
		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<CpuResourceManager> m_CpuResourceManager;
		std::shared_ptr<GpuResourceManager> m_GpuResourceManager;
		std::shared_ptr<ScriptFactory> m_ScriptFactory;

		SceneTransitionParams m_GlobalTransitionParams;

		std::mutex m_LoadSceneMutex;
		CallbackQueue<> m_MainThreadQueue;
		std::unordered_map<Guid, std::shared_ptr<Scene>> m_Scenes;
		std::unordered_map<Guid, std::shared_ptr<OneShotEvent<SceneLoadResult>>> m_LoadingScenes;

		void NotifySceneLoadFailed(const Guid& sceneGuid, std::string err);
	};
}
