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
		explicit MeshCPU(std::shared_ptr<ICPUVertexBuffer> _mesh);

	private:
		std::shared_ptr<ICPUVertexBuffer> mesh;
	};

	MeshCPU::MeshCPU(std::shared_ptr<ICPUVertexBuffer> _mesh) :
			mesh{ std::move(_mesh) }
	{
		ensure(mesh);
	}
}