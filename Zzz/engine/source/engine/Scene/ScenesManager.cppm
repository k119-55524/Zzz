#include "pch.h"
export module ScenesManager;

import Scene;
import result;
import Settings;
import ResourcesManagerGPU;

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
		explicit ScenesManager(const std::shared_ptr<ResourcesManagerGPU> resGPU);
		~ScenesManager();

		result<std::shared_ptr<Scene>> GetStartScene();

	private:
		const std::shared_ptr<ResourcesManagerGPU> m_ResGPU;
	};

	ScenesManager::ScenesManager(const std::shared_ptr<ResourcesManagerGPU> resGPU) :
		m_ResGPU{ resGPU }
	{
		ensure(m_ResGPU, ">>>>> [ScenesManager::ScenesManager()]. Resource system GPU cannot be null.");
	}

	ScenesManager::~ScenesManager()
	{
	}

	result<std::shared_ptr<Scene>> ScenesManager::GetStartScene()
	{
		std::shared_ptr<Scene> scene = std::make_shared<Scene>();
		if (!scene)
			return Unexpected(eResult::no_make_shared_ptr, L">>>>> [ScenesManager::GetStartScene()]. Failed to create Scene.");

		auto MeshGPU_DX = m_ResGPU->GetGenericMesh(MeshType::eGenericBox);
		if (!MeshGPU_DX)
			return MeshGPU_DX.error();

		return scene;
	}
}