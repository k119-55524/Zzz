
#include <format>
#include <stdexcept>

#include "Scene.h"
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
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
		std::shared_ptr<CoreCpuResourceManager> cpuResourceManager,
		std::shared_ptr<CoreGpuResourceManager> gpuResourceManager,
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

	void SceneManager::LoadSceneAsync(std::string sceneName, SceneLoadCallback onComplete)
	{
		ensure(onComplete != nullptr, "onComplete коллбэк должен быть валидным.");
		auto entryOpt = m_PackageManager->GetEntry(ePackage::Scene, sceneName);
		ensure(entryOpt.has_value(), "Сцена с именем '{}' не найдена в package.dat.", sceneName);

		LoadSceneAsync(entryOpt->GetGuid(), std::move(onComplete));
	}

	void SceneManager::LoadSceneAsync(Guid sceneGuid, SceneLoadCallback onComplete)
	{
		ensure(!sceneGuid.IsEmpty(), "GUID загружаемой сцены не может быть пустым.");
		ensure(onComplete != nullptr, "onComplete коллбэк должен быть валидным.");

		std::lock_guard lock(m_LoadSceneMutex);

		auto [it, inserted] = m_Scenes.try_emplace(sceneGuid, [this](auto task)
		{
			m_MainThreadQueue.Push(std::move(task));
		});

		it->second.readyEvent.Subscribe(std::move(onComplete));

		// Сцена уже загружена, либо в процессе загрузки
		if (!inserted)
			return;

		m_TaskDispatcher.Submit(eTaskPriority::Normal, [this, sceneGuid]()
			{
				try
				{
					auto entryOpt = m_PackageManager->GetEntry(ePackage::Scene, sceneGuid);
					ensure(entryOpt.has_value(), "Сцена с GUID '{}' не найдена в package.dat.", sceneGuid.ToString());

					auto scene = safe_make_shared<Scene>(
						sceneGuid,
						std::string(entryOpt->GetName()),
						m_CpuResourceManager,
						m_GpuResourceManager,
						m_GlobalTransitionParams
					);

					// Инициализация слоёв сцены
					scene->Initialize(*m_ScriptFactory, m_TaskDispatcher, [this, scene](std::expected<void, std::string> initRes) mutable
					{
						if (!initRes)
						{
							NotifySceneLoadFailed(scene->GetGuid(), std::move(initRes.error()));

							return;
						}

						DOut("[SceneManager::LoadSceneAsync] Собрана сцена '{}' ({}).", scene->GetName(), scene->GetGuid().ToString());

						m_MainThreadQueue.Push([this, scene = std::move(scene)]() mutable
						{
							std::lock_guard lock(m_LoadSceneMutex);
							auto it = m_Scenes.find(scene->GetGuid());
							ensure(it != m_Scenes.end(), "Запись сцены не найдена в реестре.");

							scene->InvokeStart();
							it->second = scene;
							it->second.readyEvent.Resolve(scene);
						});
					});
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
			std::lock_guard lock(m_LoadSceneMutex);
			auto it = m_Scenes.find(sceneGuid);
			if (it != m_Scenes.end())
			{
				it->second.readyEvent.Resolve(std::unexpected(err));
				m_Scenes.erase(it);
			}
		});
	}

	void SceneManager::Update(const Time& time)
	{
		// 1. Разбор отложенных задач диспетчеризации (перенос готовых сцен и вызовы Start в главном потоке)
		m_MainThreadQueue.ExecuteAll();

		// 2. Кадровое обновление всех активных сцен (выполнение пользовательских скриптов SceneScript/GameScript)
		for (auto& [guid, record] : m_Scenes)
		{
			if (record)
			{
				record.resource->Update(time);
			}
		}
	}
}
