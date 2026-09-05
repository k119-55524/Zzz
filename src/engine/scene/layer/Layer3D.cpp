#include "engine/scene/layer/Layer3D.h"
#include "core/utils/MemoryUtils.h"
#include "core/io/package/GameObjectData.h"
#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/MeshData.h"
#include "core/userscripts/ScriptFactory.h"
#include "core/userscripts/base_script/Script.h"
#include "core/userscripts/base_script/GameScript.h"

Z_SET_LOG_CATEGORY(::zzz::core::Scene);

namespace zzz
{
	Layer3D::Layer3D(std::string name)
		: m_Name(std::move(name))
		, m_Storage(::zzz::core::safe_make_unique<DefaultSceneStorage>())
	{
		m_ObjectWorld.SetStorage(m_Storage.get());
	}

	void Layer3D::Update(float dt)
	{
		if (!m_IsEnabled)
		{
			return;
		}

		m_ObjectWorld.Update(dt);
	}

	void Layer3D::PopulateObject(
		const ::zzz::core::GameObjectData& objData,
		const ::zzz::core::ScriptFactory& scriptFactory,
		::zzz::core::DataAssetsManager* dataAssetsManager)
	{
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

		// Загрузка и привязка MeshData через DataAssetsManager
		if (go->HasMesh() && dataAssetsManager != nullptr)
		{
			auto meshRes = dataAssetsManager->LoadData<::zzz::core::MeshData>(::zzz::core::eResourceType::Mesh, go->GetMeshGuid());
			if (!meshRes)
			{
				THROW_RUNTIME("Ошибка загрузки MeshData для объекта '{}' (GUID: {}): {}",
					go->GetName(), go->GetMeshGuid().ToString(), meshRes.error());
			}

			DOut("[Layer3D] Объект '{}' слоя '{}' успешно привязал MeshData ({}) из data.dat (вершин: {}, индексов: {}).",
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
