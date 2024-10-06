#include "pch.h"
#include "Scene.h"

using namespace Zzz;
using namespace Zzz::Core;
using namespace Zzz::Platforms;

Scene::Scene() :
	typeClear{e_TypeClear::Color}
{
}

void Scene::Update()
{
}

zResult Scene::Save(const zStr& path)
{
	return zResult();
}
