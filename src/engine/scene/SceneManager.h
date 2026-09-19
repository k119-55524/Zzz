#pragma once

#include "engine/tasks/TaskDispatcher.h"
#include "core/templates/CallbackQueue.h"
#include "core/scene/SceneTransitionParams.h"
#include "core/events/OneShotEvent.h"

using namespace zzz::core;
using namespace zzz::templates;

namespace zzz::engine
{
	class PackageManager;
	class CpuResourceManager;
	class GpuResourceManager;
	class Scene;

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

		void LoadSceneAsync(Guid sceneGuid, SceneLoadCallback onComplete);
		void LoadSceneAsync(std::string sceneName, SceneLoadCallback onComplete);

		void Update(const Time& time);

	private:
		TaskDispatcher& m_TaskDispatcher;
		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<CpuResourceManager> m_CpuResourceManager;
		std::shared_ptr<GpuResourceManager> m_GpuResourceManager;
		std::shared_ptr<ScriptFactory> m_ScriptFactory;

		SceneTransitionParams m_GlobalTransitionParams;

		struct SceneRecord final
		{
			std::shared_ptr<Scene> scene{ nullptr };
			core::OneShotEvent<SceneLoadResult> readyEvent;

			SceneRecord() = delete;
			explicit SceneRecord(core::OneShotEvent<SceneLoadResult>::DispatcherFunc dispatcher)
				: readyEvent(std::move(dispatcher))
			{
			}

			SceneRecord& operator=(std::shared_ptr<Scene> loadedScene)
			{
				scene = loadedScene;
				readyEvent.Resolve(std::move(loadedScene));
				return *this;
			}

			[[nodiscard]] explicit operator bool() const noexcept
			{
				return scene != nullptr;
			}
		};

		std::mutex m_LoadSceneMutex;
		CallbackQueue<> m_MainThreadQueue;
		std::unordered_map<Guid, SceneRecord> m_Scenes;
	};
}
