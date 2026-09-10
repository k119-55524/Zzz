
#include "core/utils/MemoryUtils.h"
#include "engine/scene/layer/Layer2D.h"

namespace zzz::engine
{
	Layer2D::Layer2D(
		std::string name,
		std::shared_ptr<ResourceManager> resourceManager,
		std::unique_ptr<IObjectDomain> objectDomain,
		std::unique_ptr<IEntityDomain> entityDomain,
		std::unique_ptr<ISpatialStorage> spatialStorage)
		: SpatialLayer(
			std::move(name),
			eLayerType::Layer2D,
			std::move(resourceManager),
			std::move(objectDomain),
			std::move(entityDomain),
			std::move(spatialStorage))
	{
	}
}
