#include "ObjectDomain2D.h"
#include "core/io/package/GameObjectData.h"

using namespace zzz::core;

Z_SET_LOG_CATEGORY(zzz::core::Scene);

namespace zzz::engine
{
	ObjectDomain2D::ObjectDomain2D()
		: IObjectDomain()
	{
	}

	ObjectDomain2D::~ObjectDomain2D() = default;

	void ObjectDomain2D::ValidateVisual(const VisualPayload& visual) const
	{
		switch (visual.type)
		{
		case eVisualType::None:
		case eVisualType::Sprite:
		case eVisualType::Mesh2D:
			break;
		default:
			THROW_RUNTIME("[ObjectDomain2D] Недопустимый тип визуала для Layer2D: {}", ToString(visual.type));
		}
	}

	GameObject* ObjectDomain2D::CreateObject(const GameObjectData& objData)
	{
		VisualPayload visual;
		visual.type = eVisualType::Mesh2D;
		visual.data = SimpleMeshData{
			.meshGuid = objData.GetMeshGuid(),
			.materialGuid = objData.GetMaterialGuid()
		};

		return CreateObject(objData.GetGuid(), objData.GetName(), std::move(visual));
	}

	GameObject* ObjectDomain2D::CreateObject(const Guid& guid, std::string name, VisualPayload visual)
	{
		ValidateVisual(visual);
		return RegisterObject(guid, std::move(name), std::move(visual));
	}
}
