
#include "core/utils/MemoryUtils.h"
#include "engine/resources/ResourceManager.h"
#include "engine/scene/domain/DefaultDomainFactory.h"
#include "engine/scene/storage/DefaultSpatialStorage.h"

#include "Layer3D.h"

Z_SET_LOG_CATEGORY(::zzz::core::Scene);

using namespace zzz::core;

namespace zzz::engine
{
	Layer3D::Layer3D(std::string name, std::shared_ptr<ResourceManager> resourceManager)
		: SceneTreeLayerBase(
			std::move(name),
			std::move(resourceManager),
			safe_make_unique<DefaultDomainFactory>(),
			safe_make_unique<DefaultSpatialStorage>())
	{
	}
}
