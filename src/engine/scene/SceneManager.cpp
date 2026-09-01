
#include "Scene.h"
#include "core/io/package/SceneData.h"
#include "engine/package/PackageManager.h"

#include "SceneManager.h"


Z_SET_LOG_CATEGORY(::zzz::core::Scene);

using namespace zzz::core;

namespace zzz::engine
{
	SceneManager::SceneManager(std::shared_ptr<PackageManager> packageManager, std::shared_ptr<ScriptFactory> scriptFactory) :
		m_PackageManager(std::move(packageManager)),
		m_ScriptFactory(std::move(scriptFactory))
	{
		ensure(m_PackageManager != nullptr, "PackageManager не должен быть null.");
		ensure(m_ScriptFactory != nullptr, "ScriptFactory не должен быть null.");
	}

	std::expected<std::shared_ptr<Scene>, std::string> SceneManager::LoadScene(const Guid& sceneGuid)
	{
		if (auto it = m_Scenes.find(sceneGuid); it != m_Scenes.end())
			return it->second;

		auto entryOpt = m_PackageManager->GetSceneEntryByGuid(sceneGuid);
		if (!entryOpt)
			return UNEXPECTED("Сцена с GUID '{}' не найдена в package.dat.", sceneGuid.ToString());

		auto sceneDataRes = m_PackageManager->LoadPackageDataByGuid<SceneData>(ePackage::Scene, sceneGuid);
		if (!sceneDataRes)
			return UNEXPECTED("Не удалось загрузить данные сцены '{}' ({}): {}", entryOpt->GetName(), sceneGuid.ToString(), sceneDataRes.error());

		auto scene = safe_make_shared<Scene>(sceneGuid, entryOpt->GetName(), sceneDataRes->GetSceneScriptGuids(), *m_ScriptFactory, sceneDataRes->GetClearConfig());

		m_Scenes[sceneGuid] = scene;
		scene->InvokeStart();

		DOut("[SceneManager::LoadScene] Загружена сцена '{}' ({}), скриптов: {}.", entryOpt->GetName(), sceneGuid.ToString(), sceneDataRes->GetSceneScriptGuids().size());

		return scene;
	}

	std::expected<std::shared_ptr<Scene>, std::string> SceneManager::LoadSceneByName(std::string_view sceneName)
	{
		auto entryOpt = m_PackageManager->GetSceneEntryByName(sceneName);
		if (!entryOpt)
			return UNEXPECTED("Сцена с именем '{}' не найдена в package.dat.", sceneName);

		return LoadScene(entryOpt->GetGuid());
	}

	void SceneManager::Update(const Time& time)
	{
		for (const auto& [guid, scene] : m_Scenes)
			scene->Update(time);
	}
}
