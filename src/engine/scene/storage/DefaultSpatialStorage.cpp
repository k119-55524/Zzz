
#include "engine/scene/storage/DefaultSpatialStorage.h"

namespace zzz::engine
{
	void DefaultSpatialStorage::Clear()
	{
		m_MeshNodes.clear();
	}

	SpatialHandle DefaultSpatialStorage::AddMeshNode(NodeHandle nodeHandle)
	{
		const SpatialHandle handle = static_cast<SpatialHandle>(m_MeshNodes.size());
		m_MeshNodes.push_back(nodeHandle);
		return handle;
	}
}
