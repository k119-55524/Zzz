
#include "core/utils/MemoryUtils.h"
#include "engine/resources/ResourceManager.h"
#include "engine/scene/domain/DefaultDomainFactory.h"
#include "engine/scene/storage/DefaultSpatialStorage.h"

#include "Layer3D.h"

Z_SET_LOG_CATEGORY(::zzz::core::Scene);

namespace zzz::engine
{
	Layer3D::Layer3D(std::string name, std::shared_ptr<::zzz::engine::ResourceManager> resourceManager)
		: SceneTreeLayerBase(
			std::move(name),
			std::move(resourceManager),
			::zzz::core::safe_make_unique<DefaultDomainFactory>(),
			::zzz::core::safe_make_unique<DefaultSpatialStorage>())
	{
	}
}
