
#include "core/utils/MemoryUtils.h"
#include "engine/scene/domain/MVVMDomain.h"
#include "engine/scene/domain/ObjectDomain2D.h"
#include "engine/scene/domain/ObjectDomain3D.h"
#include "engine/scene/domain/EntityDomain.h"
#include "engine/scene/storage/DefaultSpatialStorage.h"

#include "LayerSubsystemFactory.h"

using namespace zzz::core;

namespace zzz::engine
{
	std::unique_ptr<IObjectDomain> LayerSubsystemFactory::CreateObjectDomain2D() const
	{
		return safe_make_unique<ObjectDomain2D>();
	}

	std::unique_ptr<IObjectDomain> LayerSubsystemFactory::CreateObjectDomain3D() const
	{
		return safe_make_unique<ObjectDomain3D>();
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
