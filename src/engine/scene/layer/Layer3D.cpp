
#include "engine/scene/layer/Layer3D.h"
#include "core/utils/MemoryUtils.h"
#include "core/io/package/LayerData.h"
#include "core/io/package/MeshData.h"
#include "engine/resources/ResourceManager.h"
#include "core/userscripts/ScriptFactory.h"

Z_SET_LOG_CATEGORY(::zzz::core::Scene);

namespace zzz::engine
{
	Layer3D::Layer3D(std::string name, std::shared_ptr<::zzz::engine::ResourceManager> resourceManager)
		: m_Name(std::move(name))
		, m_IsVisible{ true }
		, m_Storage(::zzz::core::safe_make_unique<DefaultSceneStorage>())
		, m_ResourceManager(std::move(resourceManager))
	{
		m_ObjectWorld.SetStorage(m_Storage.get());
	}

	void Layer3D::Update(float dt)
	{
		if (!m_IsVisible)
		{
			return;
		}

		m_ObjectWorld.Update(dt);
		m_EntityWorld.Update(dt);
	}

	void Layer3D::Populate(const LayerData& layerData, const ScriptFactory& scriptFactory)
	{
		for (const auto& objData : layerData.GetObjects())
		{
			if (objData.IsEntity())
			{
				m_EntityWorld.CreateEntity(objData.GetGuid(), objData.GetName());
				continue;
			}

			auto* go = m_ObjectWorld.CreateObject(objData.GetGuid(), objData.GetName());
			if (go == nullptr)
			{
				THROW_RUNTIME("Не удалось создать GameObject '{}' в Layer3D '{}'", objData.GetName(), m_Name);
			}

			go->SetActive(objData.IsActive());
			go->GetTransform().SetLocalPosition(objData.GetPosition());
			go->GetTransform().SetLocalRotation(objData.GetRotation());
			go->GetTransform().SetLocalScale(objData.GetScale());
			go->SetMeshGuid(objData.GetMeshGuid());
			go->SetMaterialGuid(objData.GetMaterialGuid());

			for (const auto& sGuid : objData.GetScriptGuids())
			{
				auto script = scriptFactory.CreateScript(sGuid, go);
				if (script != nullptr)
				{
					go->AddScript(std::move(script));
				}
			}

			// Загрузка и привязка MeshData через ResourceManager
			if (go->HasMesh() && m_ResourceManager != nullptr)
			{
				auto meshRes = m_ResourceManager->LoadDataAsset<::zzz::core::MeshData>(go->GetMeshGuid());
				if (!meshRes)
				{
					THROW_RUNTIME("Ошибка загрузки MeshData для объекта '{}' (GUID: {}): {}",
						go->GetName(), go->GetMeshGuid().ToString(), meshRes.error());
				}

				DOut("[Layer3D] Объект '{}' слоя '{}' успешно привязал MeshData ({}) через ResourceManager (вершин: {}, индексов: {}).",
					go->GetName(), m_Name, go->GetMeshGuid().ToString(),
					meshRes->GetVertexCount(), meshRes->GetIndexCount());
			}
			else
			{
				DOut("[Layer3D] Добавлен GameObject '{}' ({}) в слой '{}'",
					go->GetName(), go->GetGuid().ToString(), m_Name);
			}
		}
	}
}
