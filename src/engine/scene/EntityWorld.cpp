#include "EntityWorld.h"
#include <logger/logger.h>

namespace zzz::engine
{
	void EntityWorld::CreateEntity(const ::zzz::core::Guid& guid, std::string_view name)
	{
		m_Entities.push_back(EntityStub{
			.guid = guid,
			.name = std::string(name)
		});

		DOut("[EntityWorld] Зарегистрирована заглушка сущности '{}' ({})", name, guid.ToString());
	}

	void EntityWorld::Update(float /*dt*/)
	{
		// Заглушка: обработка ECS-систем пока не реализована
	}
}
