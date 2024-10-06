#pragma once

#include "Scene.h"
#include "../../pch/header.h"

namespace Zzz::Core
{
	class SceneManager
	{
	public:
		SceneManager();

		void Update();

	private:
		unique_ptr<Scene> currentScene;
	};
}
