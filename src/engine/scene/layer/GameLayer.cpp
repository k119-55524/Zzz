
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
			m_NodeStorage(),
			m_ObjectDomain(std::move(objectDomain)),
			m_EntityDomain(std::move(entityDomain)),
			m_SpatialStorage(std::move(spatialStorage)),
			m_LastChangeRanges()
	{
		ensure(m_ResourceManager != nullptr, "ResourceManager не должен быть null в GameLayer.");
		ensure(m_ObjectDomain != nullptr, "ObjectDomain не должен быть null в GameLayer.");
		ensure(m_EntityDomain != nullptr, "EntityDomain не должен быть null в GameLayer.");
		ensure(m_SpatialStorage != nullptr, "SpatialStorage не должен быть null в GameLayer.");
	}

	void GameLayer::BeginFrame()
	{
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
		m_LastChangeRanges = m_NodeStorage.ResolveTransforms();
	}

	void GameLayer::Populate(const LayerData& layerData, const ScriptFactory& scriptFactory)
	{
		// 1. Всегда очищаем предыдущее состояние слоя
		m_ObjectDomain->Clear();
		m_EntityDomain->Clear();
		m_SpatialStorage->Clear();
		m_LastChangeRanges = {};

		const auto& objects = layerData.GetObjects();
		if (objects.empty())
		{
			m_NodeStorage = NodeStorage();
			return;
		}

		// 2. Формируем плоское линейное хранилище узлов сцены
		m_NodeStorage = NodeStorage(objects);

		// 3. Предварительный подсчет сущностей, объектов и мешей для устранения реаллокаций векторов
		size_t objectCount = 0;
		size_t entityCount = 0;
		size_t meshCount = 0;

		const size_t nodeCount = m_NodeStorage.GetNodeCount();
		for (size_t i = 0; i < nodeCount; ++i)
		{
			const auto& objData = objects[i];
			if (objData.IsEntity())
			{
				++entityCount;
			}
			else
			{
				++objectCount;
			}

			if (objData.HasMesh())
			{
				++meshCount;
			}
		}

		m_ObjectDomain->Reserve(objectCount);
		m_EntityDomain->Reserve(entityCount);
		m_SpatialStorage->Reserve(meshCount);

		// 4. Однопроходная регистрация в домены и пространственное хранилище
		for (NodeHandle nodeHandle = 0; nodeHandle < static_cast<NodeHandle>(nodeCount); ++nodeHandle)
		{
			const auto& objData = objects[nodeHandle];

			// Регистрация в пространственное хранилище только если есть геометрия
			SpatialHandle spHandle = kInvalidSpatialHandle;
			if (objData.HasMesh())
			{
				spHandle = m_SpatialStorage->AddMeshNode(nodeHandle);
				m_NodeStorage.SetSpatialHandle(nodeHandle, spHandle);
			}

			// Взаимоисключающая маршрутизация в домены
			if (objData.IsEntity())
			{
				const DomainHandle dHandle = m_EntityDomain->CreateEntity(nodeHandle, objData, scriptFactory, *m_ResourceManager);
				m_NodeStorage.SetDomainBinding(nodeHandle, dHandle, eNodeDomainKind::Entity);
			}
			else
			{
				const auto [dHandle, go] = m_ObjectDomain->CreateObject(objData);
				ensure(go != nullptr, "GameLayer::Populate: не удалось создать GameObject для ноды {}", nodeHandle);
				go->Initialize(objData, scriptFactory, *m_ResourceManager, &m_NodeStorage, nodeHandle);

				m_NodeStorage.SetDomainBinding(nodeHandle, dHandle, eNodeDomainKind::Object);
			}
		}

		// 5. Контрактная проверка целостности связей каждого узла
		const auto bindings = m_NodeStorage.GetBindings();
		ensure(bindings.size() == nodeCount, "GameLayer::Populate: размер bindings не совпадает с nodeCount");

		for (size_t i = 0; i < nodeCount; ++i)
		{
			const auto& binding = bindings[i];
			const auto& objData = objects[i];

			ensure(binding.domainHandle != kInvalidDomainHandle, "GameLayer::Populate: узел {} не имеет привязки к домену", i);
			ensure(binding.domainKind != eNodeDomainKind::None, "GameLayer::Populate: узел {} имеет eNodeDomainKind::None", i);

			if (objData.HasMesh())
			{
				ensure(binding.spatialHandle != kInvalidSpatialHandle, "GameLayer::Populate: узел {} с мешем не зарегистрирован в spatial", i);
			}
			else
			{
				ensure(binding.spatialHandle == kInvalidSpatialHandle, "GameLayer::Populate: узел {} без меша имеет spatialHandle", i);
			}
		}
	}
}
