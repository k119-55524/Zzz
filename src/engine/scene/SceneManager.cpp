#include "SceneManager.h"
#include "Scene.h"
#include "engine/package/PackageManager.h"
#include "core/io/package/SceneData.h"

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
		// Отменяем отложенную выгрузку, если сцену запросили повторно раньше, чем сработал ProcessPendingUnloads().
		std::erase(m_PendingUnloads, sceneGuid);

		if (auto it = m_ActiveScenes.find(sceneGuid); it != m_ActiveScenes.end())
			return it->second;

		auto entryOpt = m_PackageManager->GetSceneEntryByGuid(sceneGuid);
		if (!entryOpt)
			return UNEXPECTED("Сцена с GUID '{}' не найдена в package.dat.", sceneGuid.ToString());

		auto sceneDataRes = m_PackageManager->LoadPackageDataByGuid<SceneData>(ePackage::Scene, sceneGuid);
		if (!sceneDataRes)
			return UNEXPECTED("Не удалось загрузить данные сцены '{}' ({}): {}", entryOpt->GetName(), sceneGuid.ToString(), sceneDataRes.error());

		auto scene = safe_make_shared<Scene>(sceneGuid, entryOpt->GetName(), sceneDataRes->GetSceneScriptGuids(), *m_ScriptFactory);

		// Регистрируем ДО InvokeStart(): если OnStart() одного скрипта реентрантно запросит эту же (или
		// ссылающуюся на неё) сцену через LoadScene(), дедупликация должна найти её уже здесь, а не начать
		// повторную загрузку / повторный InvokeStart().
		m_ActiveScenes[sceneGuid] = scene;
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

	void SceneManager::UnloadScene(const Guid& sceneGuid)
	{
		if (!m_ActiveScenes.contains(sceneGuid))
			return;

		if (std::find(m_PendingUnloads.begin(), m_PendingUnloads.end(), sceneGuid) == m_PendingUnloads.end())
			m_PendingUnloads.push_back(sceneGuid);
	}

	void SceneManager::ProcessPendingUnloads()
	{
		if (m_PendingUnloads.empty())
			return;

		for (const auto& guid : m_PendingUnloads)
		{
			auto it = m_ActiveScenes.find(guid);
			if (it == m_ActiveScenes.end())
				continue;

			it->second->InvokeDestroy();
			DOut("[SceneManager::ProcessPendingUnloads] Выгружена сцена '{}' ({}).", it->second->GetName(), guid.ToString());
			m_ActiveScenes.erase(it);
		}

		m_PendingUnloads.clear();
	}

	void SceneManager::Update(const Time& time)
	{
		// Обрабатываем отложенную выгрузку ДО итерации по m_ActiveScenes - вставка/удаление из неё во время
		// самой итерации (например, если бы UnloadScene() выгружал немедленно) было бы неопределённым поведением.
		ProcessPendingUnloads();

		for (const auto& [guid, scene] : m_ActiveScenes)
			scene->Update(time);
	}
}
