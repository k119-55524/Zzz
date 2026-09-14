#pragma once

#include "core/templates/CallbackQueue.h"
#include "core/scene/SceneTransitionParams.h"
#include "engine/tasks/TaskDispatcher.h"

using namespace zzz::core;
using namespace zzz::templates;

namespace zzz::engine
{
	class PackageManager;
	class ResourceManager;
	class ResourceGarbageCollector;
	class Scene;

	class SceneManager final
	{
		Z_NO_COPY_MOVE(SceneManager);

	public:
		SceneManager() = delete;
		SceneManager(
			TaskDispatcher& taskDispatcher,
			std::shared_ptr<PackageManager> packageManager,
			std::shared_ptr<ResourceManager> resourceManager,
			std::shared_ptr<ScriptFactory> scriptFactory,
			ResourceGarbageCollector* resourceGC = nullptr);
		~SceneManager() = default;

		using SceneLoadResult = std::expected<std::shared_ptr<Scene>, std::string>;
		using SceneLoadCallback = std::function<void(SceneLoadResult)>;

		void LoadSceneAsync(Guid sceneGuid, SceneLoadCallback onComplete);
		void LoadSceneAsync(std::string sceneName, SceneLoadCallback onComplete);

		void Update(const Time& time);

	private:
		TaskDispatcher& m_TaskDispatcher;
		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<ResourceManager> m_ResourceManager;
		std::shared_ptr<ScriptFactory> m_ScriptFactory;
		ResourceGarbageCollector* m_ResourceGC{ nullptr };

		SceneTransitionParams m_GlobalTransitionParams;

		std::mutex m_LoadSceneMutex;
		CallbackQueue<> m_MainThreadQueue;
		std::unordered_map<Guid, std::shared_ptr<Scene>> m_Scenes;
	};
}
