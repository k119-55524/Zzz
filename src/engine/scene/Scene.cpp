
#include "core/utils/MemoryUtils.h"
#include "core/io/package/SceneData.h"
#include "engine/tasks/TaskDispatcher.h"
#include "engine/scene/layer/GameLayer.h"
#include "engine/scene/layer/LayerMVVM.h"
#include "core/templates/CountdownTrigger.h"
#include "core/userscripts/ScriptFactory.h"
#include "engine/resources/ResourceManager.h"
#include "engine/scene/layer/LayerSubsystemFactory.h"

#include "Scene.h"

Z_SET_LOG_CATEGORY(::zzz::core::Scene);

using namespace zzz::core;
using namespace zzz::templates;

namespace zzz::engine
{
	Scene::Scene(
		Guid guid,
		std::string name,
		std::shared_ptr<ResourceManager> resourceManager,
		SceneTransitionParams defaultTransition) :
		m_Guid(guid),
		m_Name(std::move(name)),
		m_ResourceManager(std::move(resourceManager)),
		m_TransitionParams(std::move(defaultTransition))
	{
		ensure(m_ResourceManager != nullptr, "ResourceManager не должен быть null при создании Scene.");
	}

	Scene::~Scene()
	{
		InvokeDestroy();
		DOut("[Scene::~Scene] Уничтожена сцена '{}' ({})", m_Name, m_Guid.ToString());
	}

	void Scene::Initialize(
		const ScriptFactory& scriptFactory,
		TaskDispatcher& taskDispatcher,
		std::function<void(std::expected<void, std::string>)> onLayersCreated,
		std::weak_ptr<const void> ownerToken)
	{
		ensure(onLayersCreated != nullptr, "onLayersCreated коллбэк не должен быть null при инициализации Scene.");

		if (ownerToken.expired())
		{
			ownerToken = weak_from_this();
		}

		auto sceneDataRes = m_ResourceManager->LoadSceneData(m_Guid);
		if (!sceneDataRes)
			THROW_RUNTIME("Scene '{}' ({}) не смогла загрузить SceneData: {}", m_Name, m_Guid.ToString(), sceneDataRes.error());

		// Разрешение параметров перехода на сцену
		if (sceneDataRes->GetTransitionSource() == eTransitionSource::Custom)
			m_TransitionParams = sceneDataRes->GetTransitionParams();

		// Настройки очистки поверхности и буфера глубины
		m_ClearConfig = sceneDataRes->GetClearConfig();

		// Создаём экземпляры SceneScript и инициализируем их
		for (const auto& scriptGuid : sceneDataRes->GetSceneScriptGuids())
		{
			auto script = scriptFactory.CreateSceneScript(scriptGuid);
			script->Init(&m_EventBus);
			m_Scripts.push_back(std::move(script));
		}

		// Перемещаем SceneData в shared_ptr, чтобы он жил всё время асинхронного наполнения слоёв в пуле потоков
		auto sharedSceneData = std::make_shared<SceneData>(std::move(*sceneDataRes));
		const auto& layersData = sharedSceneData->GetLayers();
		const size_t layerCount = layersData.size();
		m_Layers.resize(layerCount);

		auto firstError = std::make_shared<std::string>();
		auto errorMutex = std::make_shared<std::mutex>();

		// Создаём неблокирующий триггер завершения наполнения всех слоёв
		auto trigger = std::make_shared<CountdownTrigger>(layerCount, [firstError, onLayersCreated = std::move(onLayersCreated)]()
		{
			if (!firstError->empty())
				onLayersCreated(std::unexpected(*firstError));
			else
				onLayersCreated({});
		});

		LayerSubsystemFactory factory;
		for (size_t i = 0; i < layerCount; ++i)
		{
			const auto& layerData = layersData[i];
			switch (layerData.GetType())
			{
			case eLayerType::Layer3D:
			case eLayerType::Layer2D:
			{
				auto objectDomain = factory.CreateObjectDomain();
				auto entityDomain = factory.CreateEntityDomain();
				auto spatialStorage = factory.CreateSpatialStorage(eSpatialStorageType::Flat);

				m_Layers[i] = safe_make_unique<GameLayer>(
					layerData.GetGuid(),
					layerData.GetName(),
					layerData.GetType(),
					m_ResourceManager,
					std::move(objectDomain),
					std::move(entityDomain),
					std::move(spatialStorage));
				break;
			}
			case eLayerType::LayerMVVM:
			{
				auto mvvmDomain = factory.CreateMVVMDomain();
				m_Layers[i] = safe_make_unique<LayerMVVM>(layerData.GetGuid(), layerData.GetName(), std::move(mvvmDomain));
				break;
			}
			default:
				THROW_RUNTIME("Неизвестный eLayerType ({}) у слоя '{}' в сцене '{}'",
					ToString(layerData.GetType()), layerData.GetName(), m_Name);
			}

			// Асинхронное наполнение слоя в пуле потоков через TaskDispatcher (приоритет Normal)
			taskDispatcher.Submit(eTaskPriority::Normal, [layer = m_Layers[i].get(), layerIndex = i, &scriptFactory, sharedSceneData, trigger, firstError, errorMutex, ownerToken]()
			{
				const auto& currentLayerData = sharedSceneData->GetLayers()[layerIndex];
				try
				{
					layer->Populate(currentLayerData, scriptFactory, [trigger, firstError, errorMutex](std::expected<void, std::string> res) {
						if (!res)
						{
							std::lock_guard lock(*errorMutex);
							if (firstError->empty())
							{
								*firstError = res.error();
							}
						}
						trigger->CountDown();
					}, ownerToken);
				}
				catch (const std::exception& ex)
				{
					{
						std::lock_guard lock(*errorMutex);
						if (firstError->empty())
							*firstError = ex.what();
					}
					trigger->CountDown();
				}
				catch (...)
				{
					{
						std::lock_guard lock(*errorMutex);
						if (firstError->empty())
							*firstError = std::format("Неизвестное исключение при наполнении слоя '{}'.", currentLayerData.GetName());
					}
					trigger->CountDown();
				}
			});
		}

		DOut("[Scene::Initialize] Запущена инициализация сцены '{}' ({}), скриптов: {}, слоёв: {}",
			m_Name, m_Guid.ToString(), m_Scripts.size(), m_Layers.size());
	}

	void Scene::BeginFrame()
	{
		for (const auto& layer : m_Layers)
		{
			if (layer != nullptr)
			{
				layer->BeginFrame();
			}
		}
	}

	void Scene::Update(const Time& time)
	{
		// 1. Начало кадра для слоев (очистка dirtyTracker перед скриптами)
		BeginFrame();

		// 2. Обновление скриптов сцены
		m_EventBus.InvokeUpdate(time);

		// 3. Кадровый цикл обновления слоев сцены (скрипты объектов, ResolveTransforms)
		const float dt = time.GetDeltaTime();
		for (const auto& layer : m_Layers)
		{
			if (layer != nullptr)
			{
				layer->Update(dt);
			}
		}
	}

	void Scene::InvokeStart()
	{
		m_EventBus.InvokeStart();
	}

	void Scene::InvokeDestroy()
	{
		m_EventBus.InvokeDestroy();
	}
}

