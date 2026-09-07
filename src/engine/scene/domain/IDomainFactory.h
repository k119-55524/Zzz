#pragma once

#include <memory>

#include "engine/scene/domain/IObjectDomain.h"
#include "engine/scene/domain/IEntityDomain.h"

namespace zzz::engine
{
	/**
	 * @class IDomainFactory
	 * @brief Абстрактная фабрика для создания доменов слоя сцены.
	 */
	class IDomainFactory
	{
	public:
		virtual ~IDomainFactory() = default;

		[[nodiscard]] virtual std::unique_ptr<IObjectDomain> CreateObjectDomain() = 0;
		[[nodiscard]] virtual std::unique_ptr<IEntityDomain> CreateEntityDomain() = 0;
	};
}
