
#include "Scene.h"
#include "engine/package/PackageManager.h"
#include "engine/resources/ResourceManager.h"
#include "core/io/package/ProjectManifestData.h"
#include "engine/resources/ResourceGarbageCollector.h"

#include "SceneManager.h"

Z_SET_LOG_CATEGORY(::zzz::core::Scene);

using namespace zzz::core;

namespace zzz::engine
{
	SceneManager::SceneManager(
		TaskDispatcher& taskDispatcher,
		std::shared_ptr<PackageManager> packageManager,
		std::shared_ptr<ResourceManager> resourceManager,
		std::shared_ptr<ScriptFactory> scriptFactory,
		ResourceGarbageCollector* resourceGC) :
		m_TaskDispatcher(taskDispatcher),
		m_PackageManager(std::move(packageManager)),
		m_ResourceManager(std::move(resourceManager)),
		m_ScriptFactory(std::move(scriptFactory)),
		m_ResourceGC(resourceGC)
	{
		ensure(m_PackageManager != nullptr, "PackageManager не должен быть null.");
		ensure(m_ResourceManager != nullptr, "ResourceManager не должен быть null.");
		ensure(m_ScriptFactory != nullptr, "ScriptFactory не должен быть null.");

		m_GlobalTransitionParams = m_PackageManager->GetProjectManifestData().GetDefaultTransitionParams();
	}

	void SceneManager::LoadSceneAsync(std::string sceneName, SceneLoadCallback onComplete)
	{
		ensure(onComplete != nullptr, "onComplete коллбэк должен быть валидным.");
		auto entryOpt = m_PackageManager->GetEntry(ePackage::Scene, sceneName);
		// Наличие гарантируется сборкой ассетов в package.dat; ensure для проверки целостности при разработке
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

		m_TaskDispatcher.Submit(
			eTaskPriority::Normal,
			[this, sceneGuid, onComplete]()
			{
				std::optional<ScopedGCSuspension> gcLock;
				if (m_ResourceGC)
					gcLock.emplace(*m_ResourceGC);

				auto entryOpt = m_PackageManager->GetEntry(ePackage::Scene, sceneGuid);
				ensure(entryOpt.has_value(), "Сцена с GUID '{}' не найдена в package.dat.", sceneGuid.ToString());

				const std::string sceneName = std::string(entryOpt->GetName());

				// 1. Создание экземпляра Scene по RAII (только регистрация базовых параметров)
				auto scene = safe_make_shared<Scene>(
					sceneGuid,
					sceneName,
					m_ResourceManager,
					m_GlobalTransitionParams
				);

				std::weak_ptr<const void> ownerToken = scene;

				// 2. Инициализация слоёв сцены (по завершении переносим в основной поток)
				scene->Initialize(*m_ScriptFactory, m_TaskDispatcher, [this, scene, onComplete](std::expected<void, std::string> initRes) mutable
				{
					if (!initRes)
					{
						DOutError("[SceneManager::LoadSceneAsync] Сбой инициализации слоёв сцены '{}' ({}): {}",
							scene->GetName(), scene->GetGuid().ToString(), initRes.error());

						m_MainThreadQueue.Push([onComplete, err = std::move(initRes.error())]() mutable
						{
							if (onComplete)
							{
								onComplete(std::unexpected(err));
							}
							throw std::runtime_error(err);
						});
						return;
					}

					DOut("[SceneManager::LoadSceneAsync] Собрана сцена '{}' ({}).", scene->GetName(), scene->GetGuid().ToString());

					m_MainThreadQueue.Push([this, scene = std::move(scene), onComplete = std::move(onComplete)]() mutable
					{
						m_Scenes[scene->GetGuid()] = scene;
						scene->InvokeStart();
						if (onComplete)
						{
							onComplete(scene);
						}
					});
				}, ownerToken);
			},
			// Колбэк перехвата исключений из потока моздания сцены
			[this, onComplete](std::exception_ptr ex)
			{
				// Перенаправляем исключение в очередь главного потока для перехвата в Engine::Run
				m_MainThreadQueue.Push([onComplete, ex]()
				{
					if (onComplete)
						onComplete(std::unexpected("Критическое исключение при загрузке сцены."));

					if (ex)
						std::rethrow_exception(ex);
				});
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
