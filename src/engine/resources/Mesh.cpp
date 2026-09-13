#include "Mesh.h"

using namespace zzz::core;

namespace zzz::engine
{
	Mesh::Mesh(const Guid& guid, std::string name, MeshData meshData)
		: ResourceBase(guid, eResourceType::Mesh, std::move(name))
		, m_MeshData(std::move(meshData))
	{
	}
}
