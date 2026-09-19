#pragma once

#include "core/templates/AsyncRecord.h"
#include "engine/tasks/TaskDispatcher.h"
#include "core/templates/CallbackQueue.h"
#include "engine/resources/ResourceTypes.h"
#include "core/scene/SceneTransitionParams.h"

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
		SceneManager(
			TaskDispatcher& taskDispatcher,
			std::shared_ptr<PackageManager> packageManager,
			std::shared_ptr<CoreCpuResourceManager> cpuResourceManager,
			std::shared_ptr<CoreGpuResourceManager> gpuResourceManager,
			std::shared_ptr<ScriptFactory> scriptFactory);
		~SceneManager() = default;

		using SceneLoadResult = std::expected<std::shared_ptr<Scene>, std::string>;
		using SceneLoadCallback = std::function<void(SceneLoadResult)>;
		using SceneRecord = AsyncRecord<std::shared_ptr<Scene>, SceneLoadResult>;

		void LoadSceneAsync(Guid sceneGuid, SceneLoadCallback onComplete);
		void LoadSceneAsync(std::string sceneName, SceneLoadCallback onComplete);

		void Update(const Time& time);

	private:
		TaskDispatcher& m_TaskDispatcher;
		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<CoreCpuResourceManager> m_CpuResourceManager;
		std::shared_ptr<CoreGpuResourceManager> m_GpuResourceManager;
		std::shared_ptr<ScriptFactory> m_ScriptFactory;

		SceneTransitionParams m_GlobalTransitionParams;

		std::mutex m_LoadSceneMutex;
		CallbackQueue<> m_MainThreadQueue;
		std::unordered_map<Guid, SceneRecord> m_Scenes;

		void NotifySceneLoadFailed(const Guid& sceneGuid, std::string err);
	};
}
