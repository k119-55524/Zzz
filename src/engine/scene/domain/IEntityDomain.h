#pragma once

#include <string>
#include <cstdint>

#include "core/utils/Guid.h"
#include "engine/scene/domain/ILayerDomain.h"
namespace zzz::engine
{
	/**
	 * @class IEntityDomain
	 * @brief Контракт домена управления легковесными сущностями (ECS).
	 */
	class IEntityDomain : public ILayerDomain
	{
	public:
		virtual ~IEntityDomain() override = default;

		virtual void CreateEntity(const ::zzz::core::Guid& guid, std::string_view name) = 0;
		virtual void DestroyEntity(const ::zzz::core::Guid& guid) = 0;
	};
}
