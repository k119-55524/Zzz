#include "pch.h"
export module resourcesManager;

import settings;
import cpuVertex;

export namespace zzz
{
	export class resourcesManager final
	{
	public:
		resourcesManager() = delete;
		resourcesManager(const resourcesManager&) = delete;
		resourcesManager(resourcesManager&&) = delete;
		resourcesManager& operator=(const resourcesManager&) = delete;
		resourcesManager& operator=(resourcesManager&&) = delete;
		explicit resourcesManager(const std::shared_ptr<settings> _settings);

		std::shared_ptr<ICPUVertexBuffer> GetDefaultTriangleMesh();

		~resourcesManager();

		private:
			const std::shared_ptr<settings> m_settings;
	};

	export resourcesManager::resourcesManager(const std::shared_ptr<settings> _settings)
		: m_settings{ _settings }
	{
		ensure(m_settings, ">>>>> [resourcesManager::resourcesManager()]. Settings cannot be null.");
	}

	export resourcesManager::~resourcesManager()
	{
	}

	std::shared_ptr<ICPUVertexBuffer> resourcesManager::GetDefaultTriangleMesh()
	{
		auto vb = std::make_shared<VB_P3C3>(std::initializer_list<zzz::VB_P3C3::VertexT>{
			{
				{ { 0.0f,  0.5f, 0.0f } }, // Position
				{ { 1.0f,  0.0f, 0.0f } }  // Color (Red)
			},
			{
				{ { 0.5f, -0.5f, 0.0f } },
				{ { 0.0f, 1.0f, 0.0f } }   // Green
			},
			{
				{ { -0.5f, -0.5f, 0.0f } },
				{ { 0.0f,  0.0f, 1.0f } }  // Blue
			}
		});

		return vb;
	}
}