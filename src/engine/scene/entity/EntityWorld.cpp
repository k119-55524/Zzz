#include "engine/scene/entity/EntityWorld.h"
#include <logger/logger.h>

using namespace zzz::core;

namespace zzz::engine
{
	DomainHandle EntityWorld::CreateEntity(const Guid& guid, std::string_view name, NodeHandle nodeHandle)
	{
		const DomainHandle handle = static_cast<DomainHandle>(m_Entities.size());
		m_Entities.push_back(EntityStub{
			.guid = guid,
			.name = std::string(name),
			.nodeHandle = nodeHandle
		});

		DOut("[EntityWorld] Зарегистрирована заглушка сущности '{}' ({}) для ноды {}", name, guid.ToString(), nodeHandle);
		return handle;
	}

	void EntityWorld::DestroyEntity(const Guid& guid)
	{
		std::erase_if(m_Entities, [&guid](const EntityStub& stub) {
			return stub.guid == guid;
		});
	}

	void EntityWorld::Update(float /*dt*/)
	{
		// Заглушка: обработка ECS-систем пока не реализована
	}
}
