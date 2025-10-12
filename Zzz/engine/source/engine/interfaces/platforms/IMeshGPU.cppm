#include "pch.h"
export module IMeshGPU;

import IGAPI;
import result;
import CPUMesh;
import VertexFormatMapper;
import IBaseMeshGPU_DirectX;

using namespace zzz::platforms;
using namespace zzz::platforms::directx;

export namespace zzz
{
	export class IMeshGPU :
		public IBaseMeshGPU_DirectX
	{
		friend class GPUResourcesManager;

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