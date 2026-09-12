#include "LayerSubsystemFactory.h"
#include "core/utils/MemoryUtils.h"
#include "core/utils/Ensure.h"
#include "engine/scene/domain/ObjectDomain.h"
#include "engine/scene/domain/EntityDomain.h"
#include "engine/scene/domain/MVVMDomain.h"
#include "engine/scene/storage/DefaultSpatialStorage.h"

namespace zzz::engine
{
	std::unique_ptr<IObjectDomain> LayerSubsystemFactory::CreateObjectDomain(::zzz::core::eLayerType layerType) const
	{
		ensure(layerType == ::zzz::core::eLayerType::Layer3D || layerType == ::zzz::core::eLayerType::Layer2D,
			"ObjectDomain поддерживается только для Layer3D и Layer2D.");
		return ::zzz::core::safe_make_unique<ObjectDomain>();
	}

	std::unique_ptr<IEntityDomain> LayerSubsystemFactory::CreateEntityDomain(::zzz::core::eLayerType layerType) const
	{
		ensure(layerType == ::zzz::core::eLayerType::Layer3D || layerType == ::zzz::core::eLayerType::Layer2D,
			"EntityDomain поддерживается только для Layer3D и Layer2D.");
		return ::zzz::core::safe_make_unique<EntityDomain>(layerType);
	}

	std::unique_ptr<ISpatialStorage> LayerSubsystemFactory::CreateSpatialStorage(::zzz::core::eSpatialStorageType spatialType) const
	{
		switch (spatialType)
		{
		case ::zzz::core::eSpatialStorageType::Flat:
			return ::zzz::core::safe_make_unique<DefaultSpatialStorage>();
		default:
			THROW_RUNTIME("Неподдерживаемый eSpatialStorageType ({}) в LayerSubsystemFactory", ::zzz::core::ToString(spatialType));
		}
	}

	std::unique_ptr<IMVVMDomain> LayerSubsystemFactory::CreateMVVMDomain() const
	{
		return ::zzz::core::safe_make_unique<MVVMDomain>();
	}
}
