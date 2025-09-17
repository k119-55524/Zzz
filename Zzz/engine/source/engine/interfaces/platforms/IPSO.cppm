#include "pch.h"
export module IPSO;

import IGAPI;
import IShader;
import VertexFormatMapper;

using namespace zzz::platforms;

namespace zzz
{
	export class IPSO
	{
	public:
		IPSO() = delete;
		explicit IPSO(const std::shared_ptr<IShader> _shader, const std::vector<VertexAttrDescr>& _inputLayout);
		virtual ~IPSO() = default;

#if defined(RENDER_API_D3D12)
		virtual const ComPtr<ID3D12PipelineState> GetPSO()  const noexcept = 0;
#elif defined(RENDER_API_VULKAN)
#elif defined(RENDER_API_METAL)
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

	protected:
		const std::shared_ptr<IShader> m_Shader;
		const std::vector<VertexAttrDescr>& m_InputLayout;
	};

	IPSO::IPSO(const std::shared_ptr<IShader> _shader, const std::vector<VertexAttrDescr>& _inputLayout) :
		m_Shader{ _shader },
		m_InputLayout{ _inputLayout }
	{
		ensure(m_Shader != nullptr, ">>>>> [IPSO::IPSO( ... )]. Shader pointer cannot be null.");
		ensure(!m_InputLayout.empty(), ">>>>> [IPSO::IPSO( ... )]. Input layout cannot be empty.");
	}
}