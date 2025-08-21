#include "pch.h"
export module Scene;

import ResourcesManager;

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

	};

	export Scene::Scene()
	{

	}

	export Scene::~Scene()
	{
	}
}