
#include <unordered_map>

#include "core/utils/Ensure.h"
#include "core/io/package/MeshData.h"
#include "core/io/package/LayerData.h"
#include "core/userscripts/ScriptFactory.h"
#include "engine/resources/ResourceManager.h"
#include "engine/scene/gameobject/GameObject.h"

#include "SceneTreeLayerBase.h"

Z_SET_LOG_CATEGORY(::zzz::core::Scene);

namespace zzz::engine
{
	SceneTreeLayerBase::SceneTreeLayerBase(
		std::string name,
		std::shared_ptr<ResourceManager> resourceManager,
		std::unique_ptr<IObjectDomain> objectDomain,
		std::unique_ptr<IEntityDomain> entityDomain,
		std::unique_ptr<ISpatialStorage> spatialStorage)
		: ILayer(std::move(name), std::move(objectDomain))
		, m_ResourceManager(std::move(resourceManager))
		, m_EntityDomain(std::move(entityDomain))
		, m_SpatialStorage(std::move(spatialStorage))
	{
		ensure(m_EntityDomain != nullptr, "EntityDomain не должен быть null в SceneTreeLayerBase.");
		ensure(m_EntityDomain->GetDomainType() == ::zzz::core::eObjectDomain::Entity,
			"EntityDomain в SceneTreeLayerBase должен иметь тип eObjectDomain::Entity.");
		ensure(m_SpatialStorage != nullptr, "SpatialStorage не должен быть null в SceneTreeLayerBase.");
	}

	void SceneTreeLayerBase::BeginFrame()
	{
		if (!m_IsVisible)
			return;

		m_TreeContainer.BeginFrame();
	}

	void SceneTreeLayerBase::Update(float dt)
	{
		if (!m_IsVisible)
			return;

		OnUpdateDomains(dt);
		OnUpdateSpatial();
	}

	void SceneTreeLayerBase::ApplyHandoverBarrier()
	{
		if (!m_IsVisible)
			return;

		m_TreeContainer.ApplyHandoverBarrier();
	}

	void SceneTreeLayerBase::OnUpdateDomains(float dt)
	{
		GetObjectDomain().Update(dt);
		m_EntityDomain->Update(dt);
	}

	void SceneTreeLayerBase::OnUpdateSpatial()
	{
		m_TreeContainer.ResolveTransforms();
	}

	void SceneTreeLayerBase::Populate(const LayerData& layerData, const ScriptFactory& scriptFactory)
	{
		std::unordered_map<Guid, NodeHandle> guidToHandle;

		// --- Проход 1: Создание объектов и регистрация в структурах слоя ---
		for (const auto& objData : layerData.GetObjects())
		{
			const NodeHandle handle = objData.IsEntity()
				? PopulateEntity(objData)
				: PopulateGameObject(objData, scriptFactory);

			// Общие параметры пространственного узла (и для GameObject, и для Entity)
			m_TreeContainer.SetActive(handle, objData.IsActive());
			m_TreeContainer.SetLocalPosition(handle, objData.GetPosition());
			m_TreeContainer.SetLocalRotation(handle, objData.GetRotation());
			m_TreeContainer.SetLocalScale(handle, objData.GetScale());

			const uint32_t spHandle = m_SpatialStorage->Insert(static_cast<uint64_t>(handle.index));
			m_TreeContainer.SetSpatialHandle(handle, spHandle);

			guidToHandle[objData.GetGuid()] = handle;
		}

		// --- Проход 2: Связывание иерархии ---
		for (const auto& objData : layerData.GetObjects())
		{
			const auto& parentGuid = objData.GetParentGuid();
			if (parentGuid != ::zzz::core::Guid{})
			{
				auto childIt = guidToHandle.find(objData.GetGuid());
				if (childIt != guidToHandle.end())
				{
					auto parentIt = guidToHandle.find(parentGuid);
					if (parentIt != guidToHandle.end())
					{
						m_TreeContainer.SetParent(childIt->second, parentIt->second, false);
					}
					else
					{
						DOutWarning("[SceneTreeLayerBase::Populate] Родитель с GUID '{}' не найден для объекта '{}' в слое '{}'",
							parentGuid.ToString(), objData.GetName(), m_Name);
					}
				}
			}
		}
	}

	NodeHandle SceneTreeLayerBase::PopulateEntity(const GameObjectData& objData)
	{
		m_EntityDomain->CreateEntity(objData.GetGuid(), objData.GetName());
		return m_TreeContainer.CreateNode(objData.GetName(), nullptr);
	}

	NodeHandle SceneTreeLayerBase::PopulateGameObject(const GameObjectData& objData, const ScriptFactory& scriptFactory)
	{
		GameObject* go = GetObjectDomain().CreateObject(objData.GetGuid(), objData.GetName());
		if (go == nullptr)
		{
			THROW_RUNTIME("Не удалось создать GameObject '{}' в слое '{}'", objData.GetName(), m_Name);
		}

		NodeHandle handle = m_TreeContainer.CreateNode(objData.GetName(), go);
		go->BindSceneTree(&m_TreeContainer, handle);

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

		if (m_ResourceManager != nullptr && objData.GetMeshGuid() != ::zzz::core::Guid{})
		{
			auto res = m_ResourceManager->LoadDataAsset<::zzz::core::MeshData>(objData.GetMeshGuid());
			if (res)
			{
				DOut("[SceneTreeLayerBase::Populate] Меш '{}' успешно загружен: вершин {}, треугольников {}",
					objData.GetMeshGuid().ToString(), res->GetVertexCount(), res->GetIndexCount() / 3);
			}
			else
			{
				DOutWarning("[SceneTreeLayerBase::Populate] Не удалось загрузить меш '{}': {}",
					objData.GetMeshGuid().ToString(), res.error());
			}
		}

		return handle;
	}
}
