#pragma once

#include "core/templates/ThreadPool.h"
#include "core/templates/CallbackQueue.h"
#include "core/scene/transition/SceneTransitionParams.h"

using namespace zzz::core;
using namespace zzz::templates;

namespace zzz::engine
{
	class PackageManager;
	class Scene;

	class SceneManager final
	{
		Z_NO_COPY_MOVE(SceneManager);

	public:
		SceneManager() = delete;
		SceneManager(std::shared_ptr<PackageManager> packageManager, std::shared_ptr<ScriptFactory> scriptFactory);
		~SceneManager() = default;

		using SceneLoadResult = std::expected<std::shared_ptr<Scene>, std::string>;
		using SceneLoadCallback = std::function<void(SceneLoadResult)>;

		void LoadSceneAsync(Guid sceneGuid, SceneLoadCallback onComplete);
		void LoadSceneAsync(std::string sceneName, SceneLoadCallback onComplete);

		void Update(const Time& time);

	private:
		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<ScriptFactory> m_ScriptFactory;

		SceneTransitionParams m_GlobalTransitionParams;
		std::unique_ptr<ThreadPool> m_LoadingThreadPool;

		std::mutex m_LoadSceneMutex;
		CallbackQueue<> m_MainThreadQueue;
		std::vector<std::shared_ptr<Scene>> m_Scenes;
	};
}
