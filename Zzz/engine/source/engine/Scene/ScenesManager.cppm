#include "pch.h"
export module ScenesManager;

import Scene;
import result;
import Settings;
import SceneEntitySpawner;
import GPUResourcesManager;

export namespace zzz
{
	export class ScenesManager final
	{
	public:
		ScenesManager() = delete;
		ScenesManager(const ScenesManager&) = delete;
		ScenesManager(ScenesManager&&) = delete;
		ScenesManager& operator=(const ScenesManager&) = delete;
		ScenesManager& operator=(ScenesManager&&) = delete;
		explicit ScenesManager(const std::shared_ptr<GPUResourcesManager> resGPU);
		~ScenesManager();

		result<std::shared_ptr<Scene>> GetStartScene();

	private:
		SceneEntitySpawner m_EntitySpawner;
	};

	ScenesManager::ScenesManager(const std::shared_ptr<GPUResourcesManager> resGPU) :
		m_EntitySpawner{ resGPU }
	{
	}

	ScenesManager::~ScenesManager()
	{
	}

	result<std::shared_ptr<Scene>> ScenesManager::GetStartScene()
	{
		std::shared_ptr<Scene> scene = safe_make_shared<Scene>();
		if (!scene)
			return Unexpected(eResult::no_make_shared_ptr, L">>>>> [ScenesManager::GetStartScene()]. Failed to create Scene.");

		result<std::shared_ptr<SceneEntity>> box = m_EntitySpawner.SpawnGenericBox();
		if(!box)
			return box.error();

		scene->Add(box.value());

		return scene;
	}
}