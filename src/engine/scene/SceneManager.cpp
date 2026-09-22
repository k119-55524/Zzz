
#include "Scene.h"
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
#include "core/io/package/SceneData.h"
#include "engine/package/PackageManager.h"
#include "core/io/package/ProjectManifestData.h"
#include "engine/resources/cpu/CpuResourceManager.h"
#include "engine/resources/gpu/GpuResourceManager.h"

#include "SceneManager.h"

Z_SET_LOG_CATEGORY(::zzz::core::Scene);

using namespace zzz::core;

namespace zzz::engine
{
	SceneManager::SceneManager(
		TaskDispatcher& taskDispatcher,
		std::shared_ptr<PackageManager> packageManager,
		std::shared_ptr<CpuResourceManager> cpuResourceManager,
		std::shared_ptr<GpuResourceManager> gpuResourceManager,
		std::shared_ptr<ScriptFactory> scriptFactory) :
			m_TaskDispatcher(taskDispatcher),
			m_PackageManager(std::move(packageManager)),
			m_CpuResourceManager(std::move(cpuResourceManager)),
			m_GpuResourceManager(std::move(gpuResourceManager)),
			m_ScriptFactory(std::move(scriptFactory))
	{
		ensure(m_PackageManager != nullptr, "PackageManager не должен быть null.");
		ensure(m_CpuResourceManager != nullptr, "CpuResourceManager не должен быть null.");
		ensure(m_GpuResourceManager != nullptr, "GpuResourceManager не должен быть null.");
		ensure(m_ScriptFactory != nullptr, "ScriptFactory не должен быть null.");

		m_GlobalTransitionParams = m_PackageManager->GetProjectManifestData().GetDefaultTransitionParams();
	}

	void SceneManager::LoadSceneAsync(Guid sceneGuid, SceneLoadCallback onComplete, eTaskPriority priority)
	{
		ensure(!sceneGuid.IsEmpty(), "GUID загружаемой сцены не может быть пустым.");
		ensure(onComplete != nullptr, "onComplete коллбэк должен быть валидным.");

		std::shared_ptr<OneShotEvent<SceneLoadResult>> loadEvent;

		{
			std::lock_guard lock(m_LoadSceneMutex);

			// Сцена уже полностью загружена и готова к использованию
			if (auto it = m_Scenes.find(sceneGuid); it != m_Scenes.end())
			{
				auto scene = it->second;
				m_MainThreadQueue.Push([onComplete = std::move(onComplete), scene = std::move(scene)]() mutable
				{
					onComplete(scene);
				});

				return;
			}

			// Сцена уже загружается асинхронно
			if (auto it = m_LoadingScenes.find(sceneGuid); it != m_LoadingScenes.end())
			{
				it->second->Subscribe(std::move(onComplete));

				return;
			}

			// Сцена ещё не загружается - создаём событие и регистрируем в m_LoadingScenes
			loadEvent = safe_make_shared<OneShotEvent<SceneLoadResult>>([this](auto task)
			{
				m_MainThreadQueue.Push(std::move(task));
			});

			loadEvent->Subscribe(std::move(onComplete));
			m_LoadingScenes.emplace(sceneGuid, loadEvent);
		}

		m_TaskDispatcher.Submit(priority, [this, sceneGuid, priority]()
			{
				try
				{
					auto sceneDataRes = m_PackageManager->LoadAsset<SceneData>(sceneGuid);
					ensure(sceneDataRes.has_value(), "Ошибка загрузки данных сцены '{}': {}", sceneGuid.ToString(), sceneDataRes ? "" : sceneDataRes.error());

					auto scene = safe_make_shared<Scene>(
						sceneGuid,
						sceneGuid.ToString(),
						m_CpuResourceManager,
						m_GpuResourceManager,
						m_GlobalTransitionParams
					);

					// Инициализация слоёв сцены
					scene->Initialize(std::move(*sceneDataRes), *m_ScriptFactory, m_TaskDispatcher, [this, scene](std::expected<void, std::string> initRes) mutable
					{
						if (!initRes)
						{
							NotifySceneLoadFailed(scene->GetGuid(), std::move(initRes.error()));
							return;
						}

						DOut("[SceneManager::LoadSceneAsync] Собрана сцена '{}' ({}).", scene->GetName(), scene->GetGuid().ToString());

						m_MainThreadQueue.Push([this, scene = std::move(scene)]() mutable
						{
							std::shared_ptr<OneShotEvent<SceneLoadResult>> loadEvent;

							{
								std::lock_guard lock(m_LoadSceneMutex);
								auto it = m_LoadingScenes.find(scene->GetGuid());

								loadEvent = std::move(it->second);
								m_LoadingScenes.erase(it);

								m_Scenes[scene->GetGuid()] = scene;
							}

							scene->InvokeStart();
							loadEvent->Resolve(scene);
						});
					}, priority);
				}
				catch (const std::exception& e)
				{
					NotifySceneLoadFailed(sceneGuid, e.what());
				}
				catch (...)
				{
					NotifySceneLoadFailed(sceneGuid, "Неизвестное исключение при создании сцены");
				}
			}
		);
	}

	void SceneManager::NotifySceneLoadFailed(const Guid& sceneGuid, std::string err)
	{
		DOutError("Сбой загрузки сцены ({}): {}", sceneGuid.ToString(), err);

		m_MainThreadQueue.Push([this, sceneGuid, err = std::move(err)]() mutable
		{
			std::shared_ptr<OneShotEvent<SceneLoadResult>> loadEvent;

			{
				std::lock_guard lock(m_LoadSceneMutex);
				auto it = m_LoadingScenes.find(sceneGuid);
				if (it != m_LoadingScenes.end())
				{
					loadEvent = std::move(it->second);
					m_LoadingScenes.erase(it);
				}
			}

			if (loadEvent)
			{
				loadEvent->Resolve(std::unexpected(std::move(err)));
			}
		});
	}

	void SceneManager::Update(const Time& time)
	{
		// Разбор отложенных задач диспетчеризации
		m_MainThreadQueue.ExecuteAll();

		for (auto& [guid, scene] : m_Scenes)
		{
			ensure(scene != nullptr, "Сцена с GUID '{}' равна null.", guid.ToString());

			scene->Update(time);
		}
	}
}
