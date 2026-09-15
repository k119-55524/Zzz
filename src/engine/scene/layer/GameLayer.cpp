
#include <mutex>
#include "core/templates/CountdownTrigger.h"
#include "core/utils/Ensure.h"
#include "core/io/package/MeshData.h"
#include "core/io/package/LayerData.h"
#include "core/userscripts/ScriptFactory.h"
#include "engine/resources/ResourceManager.h"
#include "engine/scene/gameobject/GameObject.h"

#include "GameLayer.h"

using namespace zzz::core;
using namespace zzz::templates;

Z_SET_LOG_CATEGORY(zzz::core::Scene);

namespace zzz::engine
{
	GameLayer::GameLayer(
		Guid guid,
		std::string name,
		eLayerType type,
		std::shared_ptr<ResourceManager> resourceManager,
		std::unique_ptr<ObjectDomain> objectDomain,
		std::unique_ptr<IEntityDomain> entityDomain,
		std::unique_ptr<ISpatialStorage> spatialStorage) :
			ILayer(guid, std::move(name), type),
			m_ResourceManager(std::move(resourceManager)),
			m_ObjectDomain(std::move(objectDomain)),
			m_EntityDomain(std::move(entityDomain)),
			m_SpatialStorage(std::move(spatialStorage))
	{
		ensure(m_ResourceManager != nullptr, "ResourceManager не должен быть null в GameLayer.");
		ensure(m_ObjectDomain != nullptr, "ObjectDomain не должен быть null в GameLayer.");
		ensure(m_EntityDomain != nullptr, "EntityDomain не должен быть null в GameLayer.");
		ensure(m_SpatialStorage != nullptr, "SpatialStorage не должен быть null в GameLayer.");
	}

	void GameLayer::Update(float dt)
	{
		if (!m_IsVisible)
			return;

		m_EntityDomain->Update(dt);
	}

	void GameLayer::Populate(
		const LayerData& layerData,
		const ScriptFactory& scriptFactory,
		std::function<void(std::expected<void, std::string>)> onReady,
		std::weak_ptr<const void> ownerToken)
	{
		// 1. Всегда очищаем предыдущее состояние слоя
		m_ObjectDomain->Clear();
		m_EntityDomain->Clear();
		m_SpatialStorage->Clear();

		const auto& objects = layerData.GetObjects();
		if (objects.empty())
		{
			m_NodeStorage = NodeStorage();
			if (onReady)
			{
				onReady({});
			}
			return;
		}

		// 2. Формируем плоское линейное хранилище узлов сцены
		m_NodeStorage = NodeStorage(objects);
		const size_t nodeCount = m_NodeStorage.GetNodeCount();

		std::vector<std::pair<NodeHandle, GameObject*>> gameObjects;
		gameObjects.reserve(nodeCount);

		// 3. Однопроходная регистрация в домены и пространственное хранилище
		for (NodeHandle nodeHandle = 0; nodeHandle < static_cast<NodeHandle>(nodeCount); ++nodeHandle)
		{
			const auto& objData = objects[nodeHandle];

			// Взаимоисключающая маршрутизация в домены
			if (objData.IsEntity())
			{
				const DomainHandle dHandle = m_EntityDomain->CreateEntity(nodeHandle, objData, scriptFactory, *m_ResourceManager);
				m_NodeStorage.SetDomainBinding(nodeHandle, dHandle, eNodeDomainKind::Entity);
			}
			else
			{
				const auto [dHandle, go] = m_ObjectDomain->CreateObject(m_NodeStorage, nodeHandle, objData);
				ensure(go != nullptr, "GameLayer::Populate: не удалось создать GameObject для ноды {}", nodeHandle);
				gameObjects.emplace_back(nodeHandle, go);

				m_NodeStorage.SetDomainBinding(nodeHandle, dHandle, eNodeDomainKind::Object);
			}

			// Регистрация в пространственное хранилище только если есть геометрия
			if (objData.HasMesh())
			{
				const SpatialHandle spHandle = m_SpatialStorage->AddMeshNode(nodeHandle);
				m_NodeStorage.SetSpatialHandle(nodeHandle, spHandle);
			}
		}

		// 4. Контрактная проверка целостности связей каждого узла
		const auto bindings = m_NodeStorage.GetBindings();
		ensure(bindings.size() == nodeCount, "GameLayer::Populate: размер bindings не совпадает с nodeCount");

		for (size_t i = 0; i < nodeCount; ++i)
		{
			const auto& binding = bindings[i];

			ensure(binding.domainHandle != kInvalidDomainHandle, "GameLayer::Populate: узел {} не имеет привязки к домену", i);
			ensure(binding.domainKind != eNodeDomainKind::None, "GameLayer::Populate: узел {} имеет eNodeDomainKind::None", i);

			if (objects[i].HasMesh())
			{
				ensure(binding.spatialHandle != kInvalidSpatialHandle, "GameLayer::Populate: узел {} с мешем не зарегистрирован в spatial", i);
			}
			else
			{
				ensure(binding.spatialHandle == kInvalidSpatialHandle, "GameLayer::Populate: узел {} без меша имеет spatialHandle", i);
			}
		}

		// 5. Асинхронная инициализация GameObjects
		if (gameObjects.empty())
		{
			if (onReady)
			{
				onReady({});
			}
			return;
		}

		auto firstError = std::make_shared<std::string>();
		auto errorMutex = std::make_shared<std::mutex>();

		auto trigger = std::make_shared<CountdownTrigger>(gameObjects.size(), [onReady = std::move(onReady), firstError]() mutable {
			if (!onReady) return;
			if (!firstError->empty())
			{
				onReady(std::unexpected(*firstError));
			}
			else
			{
				onReady({});
			}
		});

		for (const auto& [handle, go] : gameObjects)
		{
			const auto& objData = objects[handle];
			go->Initialize(
				objData,
				scriptFactory,
				*m_ResourceManager,
				[trigger, firstError, errorMutex](std::expected<void, std::string> res) {
					if (!res)
					{
						std::lock_guard lock(*errorMutex);
						if (firstError->empty())
						{
							*firstError = res.error();
						}
					}
					trigger->CountDown();
				},
				ownerToken);
		}
	}
}
