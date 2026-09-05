
#include "Scene.h"
#include "core/io/package/SceneData.h"
#include "engine/package/PackageManager.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/ProjectManifestData.h"
#include <algorithm>

#include "SceneManager.h"

Z_SET_LOG_CATEGORY(::zzz::core::Scene);

using namespace zzz::core;

namespace zzz::engine
{
	SceneManager::SceneManager(
		std::shared_ptr<PackageManager> packageManager,
		std::shared_ptr<DataAssetsManager> dataAssetsManager,
		std::shared_ptr<ScriptFactory> scriptFactory) :
		m_PackageManager(std::move(packageManager)),
		m_DataAssetsManager(std::move(dataAssetsManager)),
		m_ScriptFactory(std::move(scriptFactory)),
		m_LoadingThreadPool(safe_make_unique<zzz::templates::ThreadPool>("SceneLoader", 1))
	{
		ensure(m_PackageManager != nullptr, "PackageManager не должен быть null.");
		ensure(m_ScriptFactory != nullptr, "ScriptFactory не должен быть null.");

		m_GlobalTransitionParams = m_PackageManager->GetProjectManifestData().GetDefaultTransitionParams();
	}

	void SceneManager::LoadSceneAsync(std::string sceneName, SceneLoadCallback onComplete)
	{
		ensure(onComplete != nullptr, "onComplete коллбэк должен быть валидным.");
		auto entryOpt = m_PackageManager->GetEntryByName(ePackage::Scene, sceneName);
		ensure(entryOpt.has_value(), "Сцена с именем '{}' не найдена в package.dat.", sceneName);

		LoadSceneAsync(entryOpt->GetGuid(), std::move(onComplete));
	}

	void SceneManager::LoadSceneAsync(Guid sceneGuid, SceneLoadCallback onComplete)
	{
		ensure(!sceneGuid.IsEmpty(), "GUID загружаемой сцены не может быть пустым.");
		ensure(onComplete != nullptr, "onComplete коллбэк должен быть валидным.");

		std::lock_guard lock(m_LoadSceneMutex);

		auto it = m_Scenes.find(sceneGuid);
		if (it != m_Scenes.end())
		{
			m_MainThreadQueue.Push([onComplete = std::move(onComplete), scene = it->second]() mutable
			{
				onComplete(scene);
			});
			return;
		}

		m_LoadingThreadPool->Submit([this, sceneGuid, onComplete = std::move(onComplete)]() mutable
		{
			try
			{
				auto entryOpt = m_PackageManager->GetEntryByGuid(ePackage::Scene, sceneGuid);
				ensure(entryOpt.has_value(), "Сцена с GUID '{}' не найдена в package.dat.", sceneGuid.ToString());

				auto sceneDataRes = m_PackageManager->LoadPackageDataByGuid<SceneData>(ePackage::Scene, sceneGuid);
				if (!sceneDataRes.has_value())
				{
					std::string err = std::format("Ошибка загрузки данных сцены '{}' ({}): {}",
						entryOpt->GetName(), sceneGuid.ToString(), sceneDataRes.error());
					DOutError("[SceneManager::LoadSceneAsync] {}", err);

					m_MainThreadQueue.Push([onComplete = std::move(onComplete), err = std::move(err)]() mutable
					{
						onComplete(std::unexpected(std::move(err)));
					});
					return;
				}

				const std::string sceneName = entryOpt->GetName();
				const auto& sceneData = *sceneDataRes;

				const auto& transition = (sceneData.GetTransitionSource() == eTransitionSource::Custom)
					? sceneData.GetTransitionParams()
					: m_GlobalTransitionParams;

				auto scene = safe_make_shared<Scene>(
					sceneGuid,
					sceneName,
					sceneData.GetSceneScriptGuids(),
					*m_ScriptFactory,
					sceneData.GetClearConfig(),
					transition,
					sceneData.GetGameObjects(),
					m_DataAssetsManager
				);

				DOut("[SceneManager::LoadSceneAsync] Собрана сцена '{}' ({}), скриптов: {}.",
					sceneName, sceneGuid.ToString(), sceneData.GetSceneScriptGuids().size());

				m_MainThreadQueue.Push([this, scene = std::move(scene), onComplete = std::move(onComplete)]() mutable
				{
					m_Scenes[scene->GetGuid()] = scene;
					scene->InvokeStart();
					onComplete(scene);
				});
			}
			catch (const std::exception& ex)
			{
				std::string err = std::format("Исключение при загрузке сцены '{}': {}", sceneGuid.ToString(), ex.what());
				DOutError("[SceneManager::LoadSceneAsync] {}", err);

				m_MainThreadQueue.Push([onComplete = std::move(onComplete), err = std::move(err)]() mutable
				{
					onComplete(std::unexpected(std::move(err)));
				});
			}
			catch (...)
			{
				std::string err = std::format("Неизвестное исключение при загрузке сцены '{}'.", sceneGuid.ToString());
				DOutError("[SceneManager::LoadSceneAsync] {}", err);

				m_MainThreadQueue.Push([onComplete = std::move(onComplete), err = std::move(err)]() mutable
				{
					onComplete(std::unexpected(std::move(err)));
				});
			}
		});
	}

	void SceneManager::Update(const Time& time)
	{
		m_MainThreadQueue.ExecuteAll();

		for (const auto& [guid, scene] : m_Scenes)
		{
			if (scene != nullptr)
				scene->Update(time);
		}
	}
}
