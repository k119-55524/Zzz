#include "core/io/package/GameObjectData.h"
#include "core/io/package/MeshData.h"
#include "engine/resources/ResourceManager.h"
#include "engine/scene/domain/EntityDomain.h"

using namespace zzz::core;

Z_SET_LOG_CATEGORY(zzz::core::Scene);

namespace zzz::engine
{
	EntityDomain::EntityDomain()
		: m_World{}
	{
	}

	EntityDomain::~EntityDomain() = default;

	DomainHandle EntityDomain::CreateEntity(const Guid& guid, std::string_view name)
	{
		return m_World.CreateEntity(guid, name);
	}

	DomainHandle EntityDomain::CreateEntity(
		NodeHandle nodeHandle,
		const GameObjectData& objData,
		const ScriptFactory& scriptFactory,
		ResourceManager& resourceManager)
	{
		(void)scriptFactory;
		const DomainHandle handle = m_World.CreateEntity(objData.GetGuid(), objData.GetName(), nodeHandle);

		// Наполнение визуальными ресурсами сущности
		switch (objData.GetMeshType())
		{
		case GameObjectData::eMeshType::Multi:
		{
			for (const auto& smGuid : objData.GetSubmeshGuids())
			{
				if (smGuid.IsValid())
				{
					auto res = resourceManager.LoadDataAsset<MeshData>(smGuid);
					if (res)
					{
						DOut("[EntityDomain::CreateEntity] Сабмеш '{}' для Entity '{}' успешно загружен", smGuid.ToString(), objData.GetName());
					}
				}
			}
			break;
		}
		case GameObjectData::eMeshType::Simple:
		{
			auto res = resourceManager.LoadDataAsset<MeshData>(objData.GetMeshGuid());
			if (res)
			{
				DOut("[EntityDomain::CreateEntity] Меш '{}' для Entity '{}' успешно загружен", objData.GetMeshGuid().ToString(), objData.GetName());
			}
			break;
		}
		case GameObjectData::eMeshType::None:
		default:
			break;
		}

		return handle;
	}

	void EntityDomain::DestroyEntity(const Guid& guid)
	{
		m_World.DestroyEntity(guid);
	}

	size_t EntityDomain::GetEntityCount() const noexcept
	{
		return m_World.GetEntityCount();
	}

	const std::vector<EntityStub>& EntityDomain::GetEntities() const noexcept
	{
		return m_World.GetEntities();
	}

	void EntityDomain::Update(float dt)
	{
		m_World.Update(dt);
	}

	void EntityDomain::Clear()
	{
		m_World.Clear();
	}
}
