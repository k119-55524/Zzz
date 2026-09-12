#pragma once

#include "engine/scene/domain/IObjectDomain.h"

using namespace zzz::core;

namespace zzz::engine
{
	/**
	 * @class ObjectDomain2D
	 * @brief Домен объектов для Layer2D (разрешает None, Sprite, Mesh2D).
	 */
	class ObjectDomain2D final : public IObjectDomain
	{
		Z_NO_COPY_MOVE(ObjectDomain2D);

	public:
		ObjectDomain2D();
		~ObjectDomain2D() override;

		GameObject* CreateObject(const GameObjectData& objData) override;
		GameObject* CreateObject(const Guid& guid, std::string name, VisualPayload visual = {}) override;

	private:
		void ValidateVisual(const VisualPayload& visual) const;
	};
}
