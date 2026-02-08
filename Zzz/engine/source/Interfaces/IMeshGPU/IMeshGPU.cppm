
export module IMeshGPU;

import IGAPI;
import Result;
import CPUMesh;
import VertexFormatMapper;

namespace zzz
{
	 class GPUResManager;
}

export namespace zzz
{
	export class IMeshGPU
	{
		friend class zzz::GPUResManager;

	public:
		IMeshGPU() = delete;
		IMeshGPU(std::shared_ptr<CPUMesh> meshCPU);

		~IMeshGPU() = default;
		inline const std::vector<VertexAttrDescr>& GetInputLayout() const noexcept { return m_MeshCPU->GetInputLayout(); };
		inline size_t GetVertexCount() const noexcept { return m_VertexCount; };
		inline size_t GetIndexCount() const noexcept { return m_IndexCount; };

	protected:
		std::shared_ptr<CPUMesh> m_MeshCPU;
		size_t m_VertexCount;
		size_t m_IndexCount;

	private:
		virtual Result<> Initialize(std::shared_ptr<IGAPI> _IGAPI) = 0;
	};

	IMeshGPU::IMeshGPU(std::shared_ptr<CPUMesh> meshCPU) :
		m_MeshCPU{ meshCPU }
	{
		ensure(m_MeshCPU, ">>>>> [IMeshGPU_DX::IMeshGPU_DX()]. m_MeshCPU cannot be null.");
		m_VertexCount = meshCPU->GetMesh()->VertexCount();
		m_IndexCount = meshCPU->GetIndicies()->GetCount();
	}
}