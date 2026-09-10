
#include <unordered_map>

#include "core/utils/Ensure.h"
#include "core/io/package/MeshData.h"
#include "core/io/package/LayerData.h"
#include "core/userscripts/ScriptFactory.h"
#include "engine/resources/ResourceManager.h"
#include "engine/scene/gameobject/GameObject.h"

#include "GameLayer.h"

Z_SET_LOG_CATEGORY(::zzz::core::Scene);

namespace zzz::engine
{
	GameLayer::GameLayer(
		std::string name,
		eLayerType type,
		std::shared_ptr<ResourceManager> resourceManager,
		std::unique_ptr<IObjectDomain> objectDomain,
		std::unique_ptr<IEntityDomain> entityDomain,
		std::unique_ptr<ISpatialStorage> spatialStorage)
		: ILayer(std::move(name), type)
		, m_ResourceManager(std::move(resourceManager))
		, m_ObjectDomain(std::move(objectDomain))
		, m_EntityDomain(std::move(entityDomain))
		, m_SpatialStorage(std::move(spatialStorage))
	{
		ensure(m_ObjectDomain != nullptr, "ObjectDomain не должен быть null в GameLayer.");
		ensure(m_EntityDomain != nullptr, "EntityDomain не должен быть null в GameLayer.");
		ensure(m_SpatialStorage != nullptr, "SpatialStorage не должен быть null в GameLayer.");
	}

	void GameLayer::BeginFrame()
	{
		if (!m_IsVisible)
			return;

		m_TreeContainer.BeginFrame();
	}

	void GameLayer::Update(float dt)
	{
		if (!m_IsVisible)
			return;

		OnUpdateDomains(dt);
		OnUpdateSpatial();
	}

	void GameLayer::ApplyHandoverBarrier()
	{
		if (!m_IsVisible)
			return;

		m_TreeContainer.ApplyHandoverBarrier();
	}

	void GameLayer::OnUpdateDomains(float dt)
	{
		m_ObjectDomain->Update(dt);
		m_EntityDomain->Update(dt);
	}

	void GameLayer::OnUpdateSpatial()
	{
		m_TreeContainer.ResolveTransforms();
	}

	void GameLayer::Populate(const LayerData& layerData, const ScriptFactory& scriptFactory)
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
			if (parentGuid != Guid{})
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
						DOutWarning("[GameLayer::Populate] Родитель с GUID '{}' не найден для объекта '{}' в слое '{}'",
							parentGuid.ToString(), objData.GetName(), m_Name);
					}
				}
			}
		}
	}

	NodeHandle GameLayer::PopulateEntity(const GameObjectData& objData)
	{
		m_EntityDomain->CreateEntity(objData.GetGuid(), objData.GetName());
		return m_TreeContainer.CreateNode(objData.GetName(), nullptr);
	}

	NodeHandle GameLayer::PopulateGameObject(const GameObjectData& objData, const ScriptFactory& scriptFactory)
	{
		GameObject* go = m_ObjectDomain->AddObject(objData.GetGuid(), objData.GetName());
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
				DOut("[GameLayer::Populate] Меш '{}' успешно загружен: вершин {}, треугольников {}",
					objData.GetMeshGuid().ToString(), res->GetVertexCount(), res->GetIndexCount() / 3);
			}
			else
			{
				DOutWarning("[GameLayer::Populate] Не удалось загрузить меш '{}': {}",
					objData.GetMeshGuid().ToString(), res.error());
			}
		}

		return handle;
	}
}
