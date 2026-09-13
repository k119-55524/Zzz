#pragma once

#include <memory>

#include "core/enums/eSpatialStorageType.h"
#include "engine/scene/domain/MVVMDomain.h"
#include "engine/scene/domain/ObjectDomain.h"
#include "engine/scene/domain/IEntityDomain.h"

using namespace zzz::core;

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

		[[nodiscard]] std::unique_ptr<ObjectDomain>    CreateObjectDomain() const;
		[[nodiscard]] std::unique_ptr<IEntityDomain>   CreateEntityDomain() const;
		[[nodiscard]] std::unique_ptr<MVVMDomain>     CreateMVVMDomain() const;

		[[nodiscard]] std::unique_ptr<ISpatialStorage> CreateSpatialStorage(eSpatialStorageType spatialType) const;
	};
}
