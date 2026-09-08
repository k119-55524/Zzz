#pragma once

#include "engine/scene/domain/IDomainFactory.h"
#include "engine/scene/domain/ObjectDomain.h"
#include "engine/scene/domain/EntityDomain.h"

namespace zzz::engine
{
	/**
	 * @class DefaultDomainFactory
	 * @brief Стандартная реализация IDomainFactory для создания ObjectDomain и EntityDomain.
	 */
	class DefaultDomainFactory final : public IDomainFactory
	{
	public:
		DefaultDomainFactory() = default;
		~DefaultDomainFactory() override = default;

		[[nodiscard]] std::unique_ptr<IObjectDomain> CreateObjectDomain() override;
		[[nodiscard]] std::unique_ptr<IEntityDomain> CreateEntityDomain() override;
	};
}
