#include "pch.h"
export module IMeshGPU;

import MeshCPU;

export namespace zzz
{
	export class IMeshGPU
	{
	public:
		IMeshGPU() = delete;
		IMeshGPU(std::shared_ptr<MeshCPU> meshCPU);

		~IMeshGPU() = default;

	protected:
		std::shared_ptr<MeshCPU> m_MeshCPU;
	};

	IMeshGPU::IMeshGPU(std::shared_ptr<MeshCPU> meshCPU) :
		m_MeshCPU{ meshCPU }
	{
		ensure(m_MeshCPU, ">>>>> [IMeshGPU_DX::IMeshGPU_DX()]. m_MeshCPU cannot be null.");
	}
}