
#include "pch.h"

export module IMeshGPU;

import IGAPI;
import result;
import CPUMesh;
import IMeshGPU_DirectX;
import VertexFormatMapper;

namespace zzz
{
	 class GPUResManager;
}

export namespace zzz::engineCore
{
	export class IMeshGPU :
		public IMeshGPU_DirectX
	{
		friend class zzz::GPUResManager;

	public:
		IMeshGPU() = delete;
		IMeshGPU(std::shared_ptr<CPUMesh> meshCPU);

		~IMeshGPU() = default;
		inline const std::vector<VertexAttrDescr>& GetInputLayout() const noexcept { return m_MeshCPU->GetInputLayout(); };

	protected:
		std::shared_ptr<CPUMesh> m_MeshCPU;

	private:
		virtual result<> Initialize(std::shared_ptr<IGAPI> _IGAPI) = 0;
	};

	IMeshGPU::IMeshGPU(std::shared_ptr<CPUMesh> meshCPU) :
		m_MeshCPU{ meshCPU }
	{
		ensure(m_MeshCPU, ">>>>> [IMeshGPU_DX::IMeshGPU_DX()]. m_MeshCPU cannot be null.");
	}
}