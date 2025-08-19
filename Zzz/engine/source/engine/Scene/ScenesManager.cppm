#include "pch.h"
export module ScenesManager;

import Scene;
import result;
import settings;
import resourcesManager;

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
		explicit ScenesManager(const std::shared_ptr<resourcesManager> _resourcesManager);
		~ScenesManager();

		result<std::shared_ptr<Scene>> GetStartScene();

	private:
		const std::shared_ptr<resourcesManager> m_resourcesManager;
	};

	ScenesManager::ScenesManager(const std::shared_ptr<resourcesManager> _resourcesManager) :
		m_resourcesManager{ _resourcesManager }
	{
		ensure(m_resourcesManager, ">>>>> [ScenesManager::ScenesManager()]. Resource system cannot be null.");
	}

	ScenesManager::~ScenesManager()
	{
	}

	result<std::shared_ptr<Scene>> ScenesManager::GetStartScene()
	{
		std::shared_ptr<Scene> scene = std::make_shared<Scene>();
		if (!scene)
			return Unexpected(eResult::no_make_shared_ptr, L">>>>> [ScenesManager::GetStartScene()]. Failed to create Scene.");

		auto res = m_resourcesManager->GetDefaultTriangleMesh();
		if (!res)
			return res.error();

		return scene;
	}
}