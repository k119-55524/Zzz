
#include "core/utils/MemoryUtils.h"
#include "core/io/package/SceneData.h"
#include "engine/scene/layer/Layer3D.h"
#include "engine/scene/layer/Layer2D.h"
#include "engine/scene/layer/LayerMVVM.h"
#include "engine/scene/domain/DefaultDomainFactory.h"
#include "engine/scene/storage/DefaultSpatialStorage.h"
#include "core/userscripts/ScriptFactory.h"
#include "engine/resources/ResourceManager.h"

#include "Scene.h"

Z_SET_LOG_CATEGORY(::zzz::core::Scene);

using namespace zzz::core;

namespace zzz::engine
{
	Scene::Scene(
		Guid guid,
		std::string name,
		std::shared_ptr<ResourceManager> resourceManager,
		const ScriptFactory& scriptFactory,
		SceneTransitionParams defaultTransition) :
		m_Guid(guid),
		m_Name(std::move(name)),
		m_ResourceManager(std::move(resourceManager)),
		m_TransitionParams(std::move(defaultTransition))
	{
		ensure(m_ResourceManager != nullptr, "ResourceManager не должен быть null при создании Scene.");

		auto sceneDataRes = m_ResourceManager->LoadSceneData(m_Guid);
		if (!sceneDataRes)
			THROW_RUNTIME("Scene '{}' ({}) не смогла загрузить SceneData: {}", m_Name, m_Guid.ToString(), sceneDataRes.error());

		Initialize(*sceneDataRes, scriptFactory);
	}

	Scene::~Scene()
	{
		InvokeDestroy();
		DOut("[Scene::~Scene] Уничтожена сцена '{}' ({})", m_Name, m_Guid.ToString());
	}

	void Scene::Initialize(const SceneData& sceneData, const ScriptFactory& scriptFactory)
	{
		// Разрешение параметров перехода
		if (sceneData.GetTransitionSource() == eTransitionSource::Custom)
			m_TransitionParams = sceneData.GetTransitionParams();

		m_ClearConfig = sceneData.GetClearConfig();
		for (const auto& scriptGuid : sceneData.GetSceneScriptGuids())
		{
			auto script = scriptFactory.CreateSceneScript(scriptGuid);
			ensure(script != nullptr, "Не удалось создать экземпляр SceneScript с GUID: " + scriptGuid.ToString());
			script->Init(&m_EventBus);
			m_Scripts.push_back(std::move(script));
		}

		DefaultDomainFactory domainFactory;

		// SceneData хранит слои напрямую (LayerData: имя, тип и его собственные объекты). На каждый
		// слой заводится ровно один ILayer, а разбор объектов внутри него - целиком забота самого
		// слоя: Scene отдаёт ему LayerData целиком одним вызовом, а не гоняет по объектам сама.
		for (const auto& layerData : sceneData.GetLayers())
		{
			switch (layerData.GetType())
			{
			case eLayerType::Layer3D:
			{
				auto objectDomain = domainFactory.CreateObjectDomain();
				auto entityDomain = domainFactory.CreateEntityDomain();
				auto spatialStorage = safe_make_unique<DefaultSpatialStorage>();

				m_Layers.push_back(safe_make_unique<Layer3D>(
					layerData.GetName(),
					m_ResourceManager,
					std::move(objectDomain),
					std::move(entityDomain),
					std::move(spatialStorage)));
				break;
			}
			case eLayerType::Layer2D:
			{
				auto objectDomain = domainFactory.CreateObjectDomain();
				auto entityDomain = domainFactory.CreateEntityDomain();
				auto spatialStorage = safe_make_unique<DefaultSpatialStorage>();

				m_Layers.push_back(safe_make_unique<Layer2D>(
					layerData.GetName(),
					m_ResourceManager,
					std::move(objectDomain),
					std::move(entityDomain),
					std::move(spatialStorage)));
				break;
			}
			case eLayerType::LayerMVVM:
			{
				auto objectDomain = domainFactory.CreateObjectDomain();
				m_Layers.push_back(safe_make_unique<LayerMVVM>(layerData.GetName(), std::move(objectDomain)));
				break;
			}
			default:
				THROW_RUNTIME("Неизвестный eLayerType ({}) у слоя '{}' в сцене '{}'",
					ToString(layerData.GetType()), layerData.GetName(), m_Name);
			}

			m_Layers.back()->Populate(layerData, scriptFactory);
		}

		DOut("[Scene::Initialize] Создана сцена '{}' ({}), скриптов: {}, слоёв: {}",
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

	void Scene::ApplyHandoverBarrier()
	{
		for (const auto& layer : m_Layers)
		{
			if (layer != nullptr)
			{
				layer->ApplyHandoverBarrier();
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

