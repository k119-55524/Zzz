
#include "core/utils/MemoryUtils.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/SceneData.h"
#include "core/userscripts/ScriptFactory.h"
#include "engine/scene/layer/Layer3D.h"
#include "engine/scene/layer/LayerUI.h"
#include "engine/scene/layer/LayerMVVM.h"

#include "Scene.h"

Z_SET_LOG_CATEGORY(::zzz::core::Scene);

using namespace zzz::core;

namespace zzz::engine
{
	Scene::Scene(
		Guid guid,
		std::string name,
		const std::vector<Guid>& sceneScriptGuids,
		const ScriptFactory& scriptFactory,
		ClearConfig clearConfig,
		SceneTransitionParams transitionParams,
		const std::vector<GameObjectData>& gameObjects,
		std::shared_ptr<DataAssetsManager> dataAssetsManager) :
		m_Guid(guid),
		m_Name(std::move(name)),
		m_ClearConfig(std::move(clearConfig)),
		m_TransitionParams(std::move(transitionParams))
	{
		Initialize(sceneScriptGuids, scriptFactory, gameObjects, std::move(dataAssetsManager));
	}

	Scene::Scene(
		Guid guid,
		std::string name,
		const SceneData& sceneData,
		const ScriptFactory& scriptFactory,
		ClearConfig clearConfig,
		SceneTransitionParams transitionParams,
		std::shared_ptr<DataAssetsManager> dataAssetsManager) :
		m_Guid(guid),
		m_Name(std::move(name)),
		m_ClearConfig(std::move(clearConfig)),
		m_TransitionParams(std::move(transitionParams))
	{
		Initialize(sceneData.GetSceneScriptGuids(), scriptFactory, sceneData.GetGameObjects(), std::move(dataAssetsManager));
	}

	void Scene::Initialize(
		const std::vector<Guid>& sceneScriptGuids,
		const ScriptFactory& scriptFactory,
		const std::vector<GameObjectData>& gameObjects,
		std::shared_ptr<DataAssetsManager> dataAssetsManager)
	{
		for (const auto& scriptGuid : sceneScriptGuids)
		{
			auto script = scriptFactory.CreateSceneScript(scriptGuid);
			ensure(script != nullptr, "Не удалось создать экземпляр SceneScript с GUID: " + scriptGuid.ToString());
			script->Init(&m_EventBus);
			m_Scripts.push_back(std::move(script));
		}

		// Загрузка игровых объектов сцены с раскладкой по слоям
		for (const auto& objData : gameObjects)
		{
			std::string layerName = objData.GetLayerName().empty() ? "Default3DLayer" : objData.GetLayerName();
			ILayer* targetLayer = GetLayerByName(layerName);
			if (targetLayer == nullptr)
			{
				std::unique_ptr<ILayer> newLayer;
				switch (objData.GetLayerType())
				{
				case eLayerType::Layer3D:
					newLayer = ::zzz::core::safe_make_unique<Layer3D>(layerName);
					break;
				case eLayerType::LayerUI:
					newLayer = ::zzz::core::safe_make_unique<LayerUI>(layerName);
					break;
				case eLayerType::LayerMVVM:
					newLayer = ::zzz::core::safe_make_unique<LayerMVVM>(layerName);
					break;
				default:
					newLayer = ::zzz::core::safe_make_unique<Layer3D>(layerName);
					break;
				}

				targetLayer = newLayer.get();
				AddLayer(std::move(newLayer));
			}

			targetLayer->PopulateObject(objData, scriptFactory, dataAssetsManager.get());
		}

		if (m_Layers.empty())
		{
			AddLayer(::zzz::core::safe_make_unique<Layer3D>("Default3DLayer"));
		}

		DOut("[Scene::Initialize] Создана сцена '{}' ({}), скриптов: {}, слоёв: {}, объектов: {}",
			m_Name, m_Guid.ToString(), m_Scripts.size(), m_Layers.size(), gameObjects.size());
	}

	Scene::~Scene()
	{
		InvokeDestroy();
		DOut("[Scene::~Scene] Уничтожена сцена '{}' ({})", m_Name, m_Guid.ToString());
	}

	void Scene::AddLayer(std::unique_ptr<ILayer> layer)
	{
		if (layer == nullptr)
		{
			return;
		}

		m_Layers.push_back(std::move(layer));
	}

	ILayer* Scene::GetLayerByName(std::string_view name) const noexcept
	{
		for (const auto& layer : m_Layers)
		{
			if (layer != nullptr && layer->GetName() == name)
				return layer.get();
		}
		return nullptr;
	}

	void Scene::Update(const Time& time)
	{
		// Обновление скриптов сцены
		m_EventBus.InvokeUpdate(time);

		// Кадровый цикл обновления слоев сцены
		const float dt = time.GetDeltaTime();
		for (const auto& layer : m_Layers)
		{
			if (layer != nullptr && layer->IsEnabled())
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
