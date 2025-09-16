#include "pch.h"
export module Scene;

import SceneEntity;
import CPUResourcesManager;

export namespace zzz
{
	export class Scene final
	{
	public:
		Scene();
		Scene(const Scene&) = delete;
		Scene(Scene&&) = delete;
		Scene& operator=(const Scene&) = delete;
		Scene& operator=(Scene&&) = delete;

		~Scene();

		void Add(std::shared_ptr<SceneEntity> entity);

	private:
		std::shared_ptr<SceneEntity> m_Entity;
	};

	export Scene::Scene()
	{
	}

	export Scene::~Scene()
	{
	}

	void Scene::Add(std::shared_ptr<SceneEntity> entity)
	{
		m_Entity = entity;
	}
}