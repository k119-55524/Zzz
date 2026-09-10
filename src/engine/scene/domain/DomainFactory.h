#pragma once

#include <memory>
#include "core/enums/eLayerType.h"
#include "engine/scene/domain/IObjectDomain.h"
#include "engine/scene/domain/IEntityDomain.h"
#include "engine/scene/domain/IMVVMDomain.h"

namespace zzz::engine
{
	/**
	 * @class DomainFactory
	 * @brief Фабрика для создания доменов слоя (ObjectDomain, EntityDomain, MVVMDomain)
	 * с привязкой к типу слоя и валидацией соответствия.
	 */
	class DomainFactory
	{
	public:
		DomainFactory() = default;
		virtual ~DomainFactory() = default;

		[[nodiscard]] std::unique_ptr<IObjectDomain> CreateObjectDomain(::zzz::core::eLayerType layerType) const;
		[[nodiscard]] std::unique_ptr<IEntityDomain> CreateEntityDomain(::zzz::core::eLayerType layerType) const;
		[[nodiscard]] std::unique_ptr<IMVVMDomain>   CreateMVVMDomain() const;
	};
}
