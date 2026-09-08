#pragma once

#include <vector>
#include <string>

#include "core/utils/Guid.h"
#include "engine/scene/domain/ILayerDomain.h"

namespace zzz
{
	class GameObject;
}

namespace zzz::engine
{
	/**
	 * @class IObjectDomain
	 * @brief Контракт домена управления классическими объектами сцены (GameObject).
	 */
	class IObjectDomain : public ILayerDomain
	{
	public:
		virtual ~IObjectDomain() override = default;

		virtual ::zzz::GameObject* CreateObject(const ::zzz::core::Guid& guid, std::string name) = 0;
		virtual void DestroyObject(::zzz::GameObject* obj) = 0;
		[[nodiscard]] virtual ::zzz::GameObject* FindObjectByGuid(const ::zzz::core::Guid& guid) const noexcept = 0;
		virtual void GetAllObjects(std::vector<::zzz::GameObject*>& outObjects) const = 0;
	};
}
