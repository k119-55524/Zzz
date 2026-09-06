#include "engine/scene/layer/LayerUI.h"
#include "core/io/package/GameObjectData.h"

namespace zzz
{
	void LayerUI::Update(float dt)
	{
		if (!m_IsEnabled)
		{
			return;
		}

		m_ObjectWorld.Update(dt);
		m_EntityWorld.Update(dt);
	}

	void LayerUI::PopulateObject(
		const ::zzz::core::GameObjectData& objData,
		const ::zzz::core::ScriptFactory& /*scriptFactory*/,
		::zzz::core::DataAssetsManager* /*dataAssetsManager*/)
	{
		if (objData.IsEntity())
		{
			m_EntityWorld.CreateEntity(objData.GetGuid(), objData.GetName());
			return;
		}

		auto* go = m_ObjectWorld.CreateObject(objData.GetGuid(), objData.GetName());
		if (go != nullptr)
		{
			go->SetActive(objData.IsActive());
		}
	}
}
