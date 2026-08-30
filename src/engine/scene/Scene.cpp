#include "Scene.h"

Z_SET_LOG_CATEGORY(::zzz::core::Scene);

using namespace zzz::core;

namespace zzz::engine
{
	Scene::Scene(Guid guid, std::string name, const std::vector<Guid>& sceneScriptGuids, const ScriptFactory& scriptFactory) :
		m_Guid(guid),
		m_Name(std::move(name))
	{
		for (const auto& scriptGuid : sceneScriptGuids)
		{
			auto script = scriptFactory.CreateSceneScript(scriptGuid);
			ensure(script != nullptr, "Не удалось создать экземпляр SceneScript с GUID: " + scriptGuid.ToString());
			script->Init(&m_EventBus);
			m_Scripts.push_back(std::move(script));
		}

		DOut("[Scene::Scene] Создана сцена '{}' ({}), скриптов: {}", m_Name, m_Guid.ToString(), m_Scripts.size());
	}

	Scene::~Scene()
	{
		DOut("[Scene::~Scene] Уничтожена сцена '{}' ({})", m_Name, m_Guid.ToString());
	}

	void Scene::Update(const Time& time)
	{
		m_EventBus.InvokeUpdate(time);
	}

	void Scene::PrepareFrame([[maybe_unused]] ISurfView* surfView)
	{
		// Заготовка: реальное построение списков команд отрисовки (меши/материалы) появится вместе
		// с ResourceManager (см. Этап 6 плана пайплайна сцен и ресурсов) - пока намеренно пусто.
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
