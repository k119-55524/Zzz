#include "pch.h"
#include "SceneManager.h"

using namespace Zzz;
using namespace Zzz::Core;

SceneManager::SceneManager() :
	currentScene{ nullptr }
{
}

void SceneManager::Update()
{
	if (currentScene != nullptr)
		currentScene->Update();
}
