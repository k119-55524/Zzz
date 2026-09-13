
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
		Guid guid,
		std::string name,
		eLayerType type,
		std::shared_ptr<ResourceManager> resourceManager,
		std::unique_ptr<IObjectDomain> objectDomain,
		std::unique_ptr<IEntityDomain> entityDomain,
		std::unique_ptr<ISpatialStorage> spatialStorage) :
			ILayer(guid, std::move(name), type),
			m_ResourceManager(std::move(resourceManager)),
			m_ObjectDomain(std::move(objectDomain)),
			m_EntityDomain(std::move(entityDomain)),
			m_SpatialStorage(std::move(spatialStorage)),
			m_NodeStorage()
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
			{
				m_EntityDomain->CreateEntity(nodeIndex, objData, scriptFactory, *m_ResourceManager);
			}
			else
			{
				GameObject* go = m_ObjectDomain->CreateObject(objData);
				go->Initialize(objData, scriptFactory, *m_ResourceManager, &m_NodeStorage, nodeIndex);
			}
		}

		// Пространственное распределение
		m_SpatialStorage->Build(m_NodeStorage);
	}
}
