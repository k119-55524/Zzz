
#include <unordered_map>

#include "core/utils/Ensure.h"
#include "core/io/package/MeshData.h"
#include "core/io/package/LayerData.h"
#include "core/userscripts/ScriptFactory.h"
#include "engine/resources/ResourceManager.h"
#include "engine/scene/gameobject/GameObject.h"

#include "GameLayer.h"

using namespace zzz::core;

Z_SET_LOG_CATEGORY(zzz::core::Scene);

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
		, m_NodeStorage{}
	{
		ensure(m_ResourceManager != nullptr, "ResourceManager не должен быть null в GameLayer.");
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

		// Формируем плоский список дерева сцены
		m_NodeStorage = NodeStorage(objects);

		// Создаем игровые объекты / скрипты / ECS-сущности
		const size_t nodeCount = m_NodeStorage.GetNodeCount();
		for (zU32 nodeIndex = 0; nodeIndex < static_cast<zU32>(nodeCount); ++nodeIndex)
		{
			const zU32 dataIdx = m_NodeStorage.GetLayerObjectIndex(nodeIndex);
			const auto& objData = objects[dataIdx];

			if (objData.IsEntity())
				PopulateEntity(nodeIndex, objData);
			else
				PopulateGameObject(nodeIndex, objData, scriptFactory);
		}

		// Пространственное распределение
		m_SpatialStorage->Build(m_NodeStorage);
	}

	void GameLayer::PopulateEntity(zU32 nodeIndex, const GameObjectData& objData)
	{
		(void)nodeIndex;
		m_EntityDomain->CreateEntity(objData.GetGuid(), objData.GetName());

		// TODO (Этап 15 ECS): При полноценной реализации EntityWorld связать узел и сущность:
		// Записать nodeIndex как компонент сущности (TransformComponent / NodeComponent).

		// Точка расширения: загрузка ресурсов меша для рендера сущностей
		switch (objData.GetMeshType())
		{
		case GameObjectData::eMeshType::Multi:
		{
			for (const auto& smGuid : objData.GetSubmeshGuids())
			{
				if (smGuid.IsValid())
				{
					auto res = m_ResourceManager->LoadDataAsset<MeshData>(smGuid);
					if (res)
					{
						DOut("[GameLayer::PopulateEntity] Меш '{}' для Entity успешно загружен", smGuid.ToString());
					}
				}
			}
			break;
		}
		case GameObjectData::eMeshType::Simple:
		{
			auto res = m_ResourceManager->LoadDataAsset<MeshData>(objData.GetMeshGuid());
			if (res)
			{
				DOut("[GameLayer::PopulateEntity] Меш '{}' для Entity успешно загружен", objData.GetMeshGuid().ToString());
			}
			break;
		}
		case GameObjectData::eMeshType::None:
		default:
			break;
		}
	}

	void GameLayer::PopulateGameObject(zU32 nodeIndex, const GameObjectData& objData, const ScriptFactory& scriptFactory)
	{
		GameObject* go = m_ObjectDomain->CreateObject(objData);
		go->BindNodeStorage(&m_NodeStorage, nodeIndex);

		for (const auto& sGuid : objData.GetScriptGuids())
		{
			auto script = scriptFactory.CreateScript(sGuid, go);
			if (script != nullptr)
			{
				go->AddScript(std::move(script));
			}
		}

		switch (objData.GetMeshType())
		{
		case GameObjectData::eMeshType::Multi:
		{
			for (const auto& smGuid : objData.GetSubmeshGuids())
			{
				auto res = m_ResourceManager->LoadDataAsset<MeshData>(smGuid);
				if (res)
				{
					DOut("[GameLayer::Populate] Сабмеш '{}' успешно загружен: вершин {}, треугольников {}",
						smGuid.ToString(), res->GetVertexCount(), res->GetIndexCount() / 3);
				}
				else
					DOutWarning("[GameLayer::Populate] Не удалось загрузить сабмеш '{}': {}", smGuid.ToString(), res.error());
			}
			break;
		}
		case GameObjectData::eMeshType::Simple:
		{
			auto res = m_ResourceManager->LoadDataAsset<MeshData>(objData.GetMeshGuid());
			if (res)
			{
				DOut("[GameLayer::Populate] Меш '{}' успешно загружен: вершин {}, треугольников {}",
					objData.GetMeshGuid().ToString(), res->GetVertexCount(), res->GetIndexCount() / 3);
			}
			else
				DOutWarning("[GameLayer::Populate] Не удалось загрузить меш '{}': {}", objData.GetMeshGuid().ToString(), res.error());
			break;
		}
		case GameObjectData::eMeshType::None:
		default:
			break;
		}
	}
}
