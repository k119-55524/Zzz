
#include "core/utils/MemoryUtils.h"
#include "engine/scene/domain/MVVMDomain.h"
#include "engine/scene/domain/ObjectDomain.h"
#include "engine/scene/domain/EntityDomain.h"
#include "engine/scene/storage/DefaultSpatialStorage.h"

#include "LayerSubsystemFactory.h"

using namespace zzz::core;

namespace zzz::engine
{
	std::unique_ptr<ObjectDomain> LayerSubsystemFactory::CreateObjectDomain() const
	{
		return safe_make_unique<ObjectDomain>();
	}

	std::unique_ptr<IEntityDomain> LayerSubsystemFactory::CreateEntityDomain() const
	{
		return safe_make_unique<EntityDomain>();
	}

	std::unique_ptr<ISpatialStorage> LayerSubsystemFactory::CreateSpatialStorage(eSpatialStorageType spatialType) const
	{
		switch (spatialType)
		{
		case eSpatialStorageType::Flat:
			return safe_make_unique<DefaultSpatialStorage>();
		default:
			THROW_RUNTIME("Неподдерживаемый eSpatialStorageType ({}) в LayerSubsystemFactory", ToString(spatialType));
		}
	}

	std::unique_ptr<MVVMDomain> LayerSubsystemFactory::CreateMVVMDomain() const
	{
		return safe_make_unique<MVVMDomain>();
	}
}
