#include "ObjectDomain3D.h"
#include "core/io/package/GameObjectData.h"

using namespace zzz::core;

Z_SET_LOG_CATEGORY(zzz::core::Scene);

namespace zzz::engine
{
	ObjectDomain3D::ObjectDomain3D()
		: IObjectDomain()
	{
	}

	ObjectDomain3D::~ObjectDomain3D() = default;

	void ObjectDomain3D::ValidateVisual(const VisualPayload& visual) const
	{
		switch (visual.type)
		{
		case eVisualType::None:
		case eVisualType::SimpleMesh3D:
		case eVisualType::MultiMesh3D:
			break;
		default:
			THROW_RUNTIME("[ObjectDomain3D] Недопустимый тип визуала для Layer3D: {}", ToString(visual.type));
		}
	}

	GameObject* ObjectDomain3D::CreateObject(const GameObjectData& objData)
	{
		VisualPayload visual;
		switch (objData.GetMeshType())
		{
		case GameObjectData::eMeshType::Multi:
			visual.type = eVisualType::MultiMesh3D;
			visual.data = MultiMesh3DData{
				.submeshes = objData.GetSubmeshGuids(),
				.materials = objData.GetMaterialGuids()
			};
			break;
		case GameObjectData::eMeshType::Simple:
			visual.type = eVisualType::SimpleMesh3D;
			visual.data = SimpleMeshData{
				.meshGuid = objData.GetMeshGuid(),
				.materialGuid = objData.GetMaterialGuid()
			};
			break;
		case GameObjectData::eMeshType::None:
		default:
			break;
		}

		return CreateObject(objData.GetGuid(), objData.GetName(), std::move(visual));
	}

	GameObject* ObjectDomain3D::CreateObject(const Guid& guid, std::string name, VisualPayload visual)
	{

		return RegisterObject(guid, std::move(name), std::move(visual));
	}
}
