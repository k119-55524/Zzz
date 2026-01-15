#include "pch.h"
export module ScenesManager;

import Scene;
import Result;
import GPUResManager;
import SceneEntitySpawner;

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
		explicit ScenesManager(const std::shared_ptr<GPUResManager> resGPU);
		~ScenesManager();

		Result<std::shared_ptr<Scene>> GetDefaultScene();

	private:
		SceneEntitySpawner m_EntitySpawner;
	};

	ScenesManager::ScenesManager(const std::shared_ptr<GPUResManager> resGPU) :
		m_EntitySpawner{ resGPU }
	{
	}

	ScenesManager::~ScenesManager()
	{
	}

	Result<std::shared_ptr<Scene>> ScenesManager::GetDefaultScene()
	{
		Result<std::shared_ptr<SceneEntity>> box = m_EntitySpawner.SpawnGenericBox();
		if(!box)
			return box.error();

		Result<std::shared_ptr<Scene>> scene = safe_make_shared<Scene>();
		if (!scene)
			return scene.error();

		scene.value()->Add(box.value());

		return scene;
	}
}