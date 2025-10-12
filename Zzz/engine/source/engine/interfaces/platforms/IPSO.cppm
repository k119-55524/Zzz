#include "pch.h"
export module IPSO;

import IGAPI;
import IShader;
import IBasePSO_DirectX;
import VertexFormatMapper;

using namespace zzz::platforms;
using namespace zzz::platforms::directx;

namespace zzz
{
	export class IPSO :
		public IBasePSO_DirectX
	{
	public:
		IPSO() = delete;
		explicit IPSO(const std::shared_ptr<IShader> _shader, const std::vector<VertexAttrDescr>& _inputLayout);
		virtual ~IPSO() = default;

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