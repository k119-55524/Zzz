#include "pch.h"
export module scenesManager;

import scene;
import result;
import settings;
import resourcesManager;

export namespace zzz
{
	export class scenesManager final
	{
	public:
		scenesManager() = delete;
		scenesManager(const scenesManager&) = delete;
		scenesManager(scenesManager&&) = delete;
		scenesManager& operator=(const scenesManager&) = delete;
		scenesManager& operator=(scenesManager&&) = delete;
		explicit scenesManager(const std::shared_ptr<resourcesManager> _resourcesManager);
		~scenesManager();

		result<std::shared_ptr<scene>> GetStartScene();

	private:
		const std::shared_ptr<resourcesManager> m_resourcesManager;
	};

	scenesManager::scenesManager(const std::shared_ptr<resourcesManager> _resourcesManager) :
		m_resourcesManager{ _resourcesManager }
	{
		ensure(m_resourcesManager, ">>>>> [scenesManager::scenesManager()]. Resource system cannot be null.");
	}

	scenesManager::~scenesManager()
	{
	}

	result<std::shared_ptr<scene>> scenesManager::GetStartScene()
	{
		std::shared_ptr<scene> m_scene = std::make_shared<scene>();
		if (!m_scene)
			return Unexpected(eResult::failure, L">>>>> [scenesManager::GetStartScene()]. Failed to create scene.");

		m_resourcesManager->GetDefaultTriangleMesh();

		return m_scene;
	}
}