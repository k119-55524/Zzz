#include "DomainFactory.h"
#include "core/utils/MemoryUtils.h"
#include "core/utils/Ensure.h"
#include "engine/scene/domain/ObjectDomain.h"
#include "engine/scene/domain/EntityDomain.h"
#include "engine/scene/domain/MVVMDomain.h"

namespace zzz::engine
{
	std::unique_ptr<IObjectDomain> DomainFactory::CreateObjectDomain(::zzz::core::eLayerType layerType) const
	{
		ensure(layerType == ::zzz::core::eLayerType::Layer3D || layerType == ::zzz::core::eLayerType::Layer2D,
			"ObjectDomain поддерживается только для Layer3D и Layer2D.");
		return ::zzz::core::safe_make_unique<ObjectDomain>();
	}

	std::unique_ptr<IEntityDomain> DomainFactory::CreateEntityDomain(::zzz::core::eLayerType layerType) const
	{
		ensure(layerType == ::zzz::core::eLayerType::Layer3D || layerType == ::zzz::core::eLayerType::Layer2D,
			"EntityDomain поддерживается только для Layer3D и Layer2D.");
		return ::zzz::core::safe_make_unique<EntityDomain>(layerType);
	}

	std::unique_ptr<IMVVMDomain> DomainFactory::CreateMVVMDomain() const
	{
		return ::zzz::core::safe_make_unique<MVVMDomain>();
	}
}
