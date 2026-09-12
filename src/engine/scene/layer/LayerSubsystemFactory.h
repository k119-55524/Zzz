#pragma once

#include <memory>
#include "core/enums/eLayerType.h"
#include "core/enums/eSpatialStorageType.h"
#include "engine/scene/domain/IObjectDomain.h"
#include "engine/scene/domain/IEntityDomain.h"
#include "engine/scene/domain/IMVVMDomain.h"

namespace zzz::engine
{
	class ISpatialStorage;

	/**
	 * @class LayerSubsystemFactory
	 * @brief Фабрика для создания доменов и подсистем слоя (ObjectDomain, EntityDomain, MVVMDomain, SpatialStorage)
	 * с привязкой к типу слоя и валидацией соответствия.
	 */
	class LayerSubsystemFactory
	{
	public:
		LayerSubsystemFactory() = default;
		virtual ~LayerSubsystemFactory() = default;

		[[nodiscard]] std::unique_ptr<IObjectDomain>   CreateObjectDomain(::zzz::core::eLayerType layerType) const;
		[[nodiscard]] std::unique_ptr<IEntityDomain>   CreateEntityDomain(::zzz::core::eLayerType layerType) const;
		[[nodiscard]] std::unique_ptr<ISpatialStorage> CreateSpatialStorage(::zzz::core::eSpatialStorageType spatialType) const;
		[[nodiscard]] std::unique_ptr<IMVVMDomain>     CreateMVVMDomain() const;
	};
}
