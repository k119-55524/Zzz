
#include "core/utils/MemoryUtils.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/userscripts/ScriptFactory.h"

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
			if (objData.IsEntity())
			{
				m_EntityWorld.CreateEntity(objData.GetGuid(), objData.GetName());
				continue;
			}

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
				case eLayerType::LayerMVVM:
					THROW_RUNTIME("Тип слоя '{}' пока не поддерживается", ToString(objData.GetLayerType()));
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

		DOut("[Scene::Scene] Создана сцена '{}' ({}), скриптов: {}, слоёв: {}, объектов: {}",
			m_Name, m_Guid.ToString(), m_Scripts.size(), m_Layers.size(), gameObjects.size());
	}

	Scene::~Scene()
	{
		DOut("[Scene::~Scene] Уничтожена сцена '{}' ({})", m_Name, m_Guid.ToString());
	}

	void Scene::AddLayer(std::unique_ptr<ILayer> layer)
	{
		if (layer == nullptr)
		{
			return;
		}

		if (layer->GetType() == eLayerType::Layer3D && m_Layer3D == nullptr)
		{
			m_Layer3D = static_cast<Layer3D*>(layer.get());
		}

		m_Layers.push_back(std::move(layer));
	}

	Layer3D* Scene::GetLayer3D() const noexcept
	{
		return m_Layer3D;
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

		// Кадровый цикл ECS-мира сущностей
		m_EntityWorld.Update(dt);
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
