
#include "core/utils/MemoryUtils.h"

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
		SceneTransitionParams transitionParams) :
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

		// По умолчанию каждая сцена имеет базовый 3D слой
		AddLayer(::zzz::core::safe_make_unique<Layer3D>("Default3DLayer"));

		DOut("[Scene::Scene] Создана сцена '{}' ({}), скриптов: {}, слоёв: {}", m_Name, m_Guid.ToString(), m_Scripts.size(), m_Layers.size());
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
