#include "Mesh.h"

namespace zzz::engine
{
	Mesh::Mesh(const ::zzz::core::Guid& guid, std::string name, ::zzz::core::MeshData meshData)
		: ResourceBase(guid, ::zzz::core::eResourceType::Mesh, std::move(name))
		, m_MeshData(std::move(meshData))
	{
	}
}
