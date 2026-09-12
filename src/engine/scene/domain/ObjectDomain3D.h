#pragma once

#include "engine/scene/domain/IObjectDomain.h"

using namespace zzz::core;

namespace zzz::engine
{
	/**
	 * @class ObjectDomain3D
	 * @brief Домен объектов для Layer3D (разрешает None, SimpleMesh3D, MultiMesh3D).
	 */
	class ObjectDomain3D final : public IObjectDomain
	{
		Z_NO_COPY_MOVE(ObjectDomain3D);

	public:
		ObjectDomain3D();
		~ObjectDomain3D() override;

		GameObject* CreateObject(const GameObjectData& objData) override;

	protected:
		GameObject* CreateObject(const Guid& guid, std::string name, VisualPayload visual = {}) override;

	private:
		void ValidateVisual(const VisualPayload& visual) const;
	};
}
