#pragma once

#include <vector>
#include <string>

#include "core/utils/Guid.h"
#include "engine/scene/domain/ILayerDomain.h"

namespace zzz::engine
{
	class GameObject;

	/**
	 * @class IObjectDomain
	 * @brief Контракт домена управления классическими объектами сцены (GameObject).
	 */
	class IObjectDomain : public ILayerDomain
	{
	public:
		virtual ~IObjectDomain() override = default;

		virtual GameObject* CreateObject(const ::zzz::core::Guid& guid, std::string name) = 0;
		[[nodiscard]] virtual GameObject* FindObjectByGuid(const ::zzz::core::Guid& guid) const noexcept = 0;
		[[nodiscard]] virtual GameObject* FindObjectByName(std::string_view name) const noexcept = 0;
	};
}
