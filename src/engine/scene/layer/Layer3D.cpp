
#include "engine/resources/ResourceManager.h"

#include "Layer3D.h"

Z_SET_LOG_CATEGORY(::zzz::core::Scene);

using namespace zzz::core;

namespace zzz::engine
{
	Layer3D::Layer3D(
		std::string name,
		std::shared_ptr<ResourceManager> resourceManager,
		std::unique_ptr<IObjectDomain> objectDomain,
		std::unique_ptr<IEntityDomain> entityDomain,
		std::unique_ptr<ISpatialStorage> spatialStorage)
		: SpatialLayer(
			std::move(name),
			eLayerType::Layer3D,
			std::move(resourceManager),
			std::move(objectDomain),
			std::move(entityDomain),
			std::move(spatialStorage))
	{
	}
}
