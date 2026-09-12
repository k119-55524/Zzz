
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

		m_NodeStorage.BeginFrame();
	}

	void GameLayer::Update(float dt)
	{
		if (!m_IsVisible)
			return;

		OnUpdateDomains(dt);
		OnUpdateSpatial();
	}

	void GameLayer::OnUpdateDomains(float dt)
	{
		m_ObjectDomain->Update(dt);
		m_EntityDomain->Update(dt);
	}

	void GameLayer::OnUpdateSpatial()
	{
		m_NodeStorage.ResolveTransforms();
	}

	void GameLayer::Populate(const LayerData& layerData, const ScriptFactory& scriptFactory)
	{
		const auto& objects = layerData.GetObjects();
		if (objects.empty())
			return;

		// Шаг 1: Формируем плоский список сцены
		m_NodeStorage = NodeStorage(objects);

		// Шаг 2: Создаем игровые объекты / скрипты / ECS-сущности
		const size_t nodeCount = m_NodeStorage.GetNodeCount();
		for (uint32_t i = 0; i < static_cast<uint32_t>(nodeCount); ++i)
		{
			const NodeHandle handle = m_NodeStorage.GetHandle(i);
			const uint32_t dataIdx = m_NodeStorage.GetLayerObjectIndex(handle);
			const auto& objData = objects[dataIdx];

			if (m_NodeStorage.GetNodeType(handle) == SceneNodeType::Entity)
				PopulateEntity(handle, objData);
			else
				PopulateGameObject(handle, objData, scriptFactory);
		}

		// Шаг 3: Пространственный индекс для рендера/выборки (Spatial Index)
		// Передаем NodeStorage с уже рассчитанными мировыми матрицами/позициями
		m_SpatialStorage->Build(m_NodeStorage);
	}

	void GameLayer::PopulateEntity(NodeHandle handle, const GameObjectData& objData)
	{
		(void)handle;
		m_EntityDomain->CreateEntity(objData.GetGuid(), objData.GetName());

		// TODO (Этап 15 ECS): При полноценной реализации EntityWorld связать узел и сущность:
		// 1. Записать NodeHandle как компонент сущности (TransformComponent / NodeComponent).
		// 2. Записать полученный uint32_t entityId обратно в NodeMetadata узла:
		//    m_NodeStorage.SetEntityId(handle, entityId);

		// Точка расширения: загрузка ресурсов меша для рендера сущностей
		if (m_ResourceManager != nullptr && objData.GetMeshGuid() != Guid{})
		{
			auto res = m_ResourceManager->LoadDataAsset<MeshData>(objData.GetMeshGuid());
			if (res)
			{
				DOut("[GameLayer::PopulateEntity] Меш '{}' для Entity успешно загружен", objData.GetMeshGuid().ToString());
			}
		}
	}

	void GameLayer::PopulateGameObject(NodeHandle handle, const GameObjectData& objData, const ScriptFactory& scriptFactory)
	{
		GameObject* go = m_ObjectDomain->CreateObject(objData.GetGuid(), objData.GetName());
		go->BindNodeStorage(&m_NodeStorage, handle);
		m_NodeStorage.SetNodeOwner(handle, go);

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

		if (m_ResourceManager != nullptr && objData.GetMeshGuid() != Guid{})
		{
			auto res = m_ResourceManager->LoadDataAsset<MeshData>(objData.GetMeshGuid());
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
	}
}
