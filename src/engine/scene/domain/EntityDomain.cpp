#include "engine/scene/domain/EntityDomain.h"

namespace zzz::engine
{
	EntityDomain::EntityDomain()
		: m_World{}
	{
	}

	EntityDomain::~EntityDomain() = default;

	void EntityDomain::CreateEntity(const ::zzz::core::Guid& guid, std::string_view name)
	{
		m_World.CreateEntity(guid, name);
	}

	void EntityDomain::DestroyEntity(const ::zzz::core::Guid& guid)
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
		for (const auto& entity : m_World.GetEntities())
		{
			m_World.DestroyEntity(entity.guid);
		}
	}
}
