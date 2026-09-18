#include "core/io/package/GameObjectData.h"
#include "core/io/package/MeshData.h"
#include "engine/resources/cpu/CpuResourceManager.h"
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
		CpuResourceManager& resourceManager)
	{
		(void)scriptFactory;
		const DomainHandle handle = m_World.CreateEntity(objData.GetGuid(), objData.GetName(), nodeHandle);

		// Наполнение визуальными ресурсами сущности
		for (const auto& meshGuid : objData.GetMeshGuids())
		{
			if (meshGuid.IsValid())
			{
				auto res = resourceManager.LoadDataAsset<MeshData>(meshGuid);
				if (res)
				{
					DOut("[EntityDomain::CreateEntity] Меш '{}' для Entity '{}' успешно загружен", meshGuid.ToString(), objData.GetName());
				}
			}
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
