#include "pch.h"
export module MeshCPU;

import MeshData;

export namespace zzz
{
	export class MeshCPU final
	{
	public:
		MeshCPU() = delete;
		MeshCPU(const MeshCPU&) = delete;
		MeshCPU(MeshCPU&&) = delete;
		MeshCPU& operator=(const MeshCPU&) = delete;
		MeshCPU& operator=(MeshCPU&&) = delete;
		explicit MeshCPU(std::shared_ptr<ICPUVertexBuffer> _mesh, std::shared_ptr<std::vector<zU16>> _indicies = nullptr);

	private:
		std::shared_ptr<ICPUVertexBuffer> mesh;
	};

	MeshCPU::MeshCPU(std::shared_ptr<ICPUVertexBuffer> _mesh, std::shared_ptr<std::vector<zU16>> _indicies) :
			mesh{ std::move(_mesh) }
	{
		ensure(mesh);
	}
}