#include "pch.h"
export module Scene;

import IMeshGPU;
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

		void AddMesh(std::shared_ptr<IMeshGPU> _mesh);

	private:
		std::shared_ptr<IMeshGPU> mesh;
	};

	export Scene::Scene()
	{
	}

	export Scene::~Scene()
	{
	}

	void Scene::AddMesh(std::shared_ptr<IMeshGPU> _mesh)
	{
		mesh = _mesh;
	}
}