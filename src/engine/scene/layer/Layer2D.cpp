#include "engine/scene/layer/Layer2D.h"
#include "engine/scene/domain/DefaultDomainFactory.h"
#include "engine/scene/storage/DefaultSpatialStorage.h"
#include "core/utils/MemoryUtils.h"

namespace zzz::engine
{
	Layer2D::Layer2D(std::string name, std::shared_ptr<::zzz::engine::ResourceManager> resourceManager)
		: SceneTreeLayerBase(
			std::move(name),
			std::move(resourceManager),
			::zzz::core::safe_make_unique<DefaultDomainFactory>(),
			::zzz::core::safe_make_unique<DefaultSpatialStorage>())
	{
	}
}
