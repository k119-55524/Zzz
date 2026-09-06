#include "engine/scene/layer/LayerMVVM.h"
#include "core/io/package/LayerData.h"

namespace zzz
{
	void LayerMVVM::Update(float dt)
	{
		if (!m_IsVisible)
		{
			return;
		}

		m_ObjectWorld.Update(dt);
		m_EntityWorld.Update(dt);
	}

	void LayerMVVM::Populate(
		const ::zzz::core::LayerData& layerData,
		const ::zzz::core::ScriptFactory& /*scriptFactory*/)
	{
		for (const auto& objData : layerData.GetObjects())
		{
			if (objData.IsEntity())
			{
				m_EntityWorld.CreateEntity(objData.GetGuid(), objData.GetName());
				continue;
			}

			auto* go = m_ObjectWorld.CreateObject(objData.GetGuid(), objData.GetName());
			if (go != nullptr)
			{
				go->SetActive(objData.IsActive());
			}
		}
	}
}
