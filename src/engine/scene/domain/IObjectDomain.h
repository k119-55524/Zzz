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

		virtual GameObject* AddObject(const ::zzz::core::Guid& guid, std::string name) = 0;
		[[nodiscard]] virtual GameObject* FindObjectByGuid(const ::zzz::core::Guid& guid) const noexcept = 0;
		virtual void GetAllObjects(std::vector<GameObject*>& outObjects) const = 0;
	};
}
