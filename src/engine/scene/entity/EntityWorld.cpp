#include "engine/scene/entity/EntityWorld.h"
#include <logger/logger.h>

using namespace zzz::core;

namespace zzz::engine
{
	void EntityWorld::CreateEntity(const Guid& guid, std::string_view name)
	{
		m_Entities.push_back(EntityStub{
			.guid = guid,
			.name = std::string(name)
		});

		DOut("[EntityWorld] Зарегистрирована заглушка сущности '{}' ({})", name, guid.ToString());
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
