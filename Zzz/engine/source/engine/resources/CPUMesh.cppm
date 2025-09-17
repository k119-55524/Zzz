#include "pch.h"
export module CPUMesh;

import CPUIndexBuffer;
import CPUVertexBuffer;
import VertexFormatMapper;

export namespace zzz
{
	export class CPUMesh final
	{
	public:
		CPUMesh() = delete;
		CPUMesh(const CPUMesh&) = delete;
		CPUMesh(CPUMesh&&) = delete;
		CPUMesh& operator=(const CPUMesh&) = delete;
		CPUMesh& operator=(CPUMesh&&) = delete;
		explicit CPUMesh(std::shared_ptr<ICPUVertexBuffer> _mesh, std::shared_ptr<ICPUIndexBuffer> _indicies = nullptr);

		inline std::shared_ptr<ICPUVertexBuffer> GetMesh() const noexcept { return m_Mesh; };
		inline std::shared_ptr<ICPUIndexBuffer> GetIndicies() const noexcept { return m_Indicies; };
		inline const std::vector<VertexAttrDescr>& GetInputLayout() const noexcept { return m_InputLayout; };

	private:
		std::shared_ptr<ICPUVertexBuffer> m_Mesh;
		std::shared_ptr<ICPUIndexBuffer> m_Indicies;
		std::vector<VertexAttrDescr> m_InputLayout;
	};

	CPUMesh::CPUMesh(std::shared_ptr<ICPUVertexBuffer> _mesh, std::shared_ptr<ICPUIndexBuffer> _indicies) :
			m_Mesh{ std::move(_mesh) },
			m_Indicies{ std::move(_indicies) }
	{
		ensure(m_Mesh);

		m_InputLayout = VertexFormatMapper::GetInputLayout(*m_Mesh);

		int i = 0;
		i++;
	}
}