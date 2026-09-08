#include "engine/scene/domain/DefaultDomainFactory.h"
#include "core/utils/MemoryUtils.h"

namespace zzz::engine
{
	std::unique_ptr<IObjectDomain> DefaultDomainFactory::CreateObjectDomain()
	{
		return ::zzz::core::safe_make_unique<ObjectDomain>();
	}

	std::unique_ptr<IEntityDomain> DefaultDomainFactory::CreateEntityDomain()
	{
		return ::zzz::core::safe_make_unique<EntityDomain>();
	}
}
