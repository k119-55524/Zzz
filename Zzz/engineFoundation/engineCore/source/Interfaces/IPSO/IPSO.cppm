
export module IPSO;

import IGAPI;
import IShader;
import PrimitiveTopology;
import VertexFormatMapper;

using namespace zzz::directx;

namespace zzz
{
	export class IPSO
	{
		Z_NO_COPY_MOVE(IPSO);

	public:
		IPSO() = delete;
		explicit IPSO(const std::shared_ptr<IShader> _shader, const std::vector<VertexAttrDescr>& _inputLayout, PrimitiveTopology _topo);
		virtual ~IPSO() = default;

		const PrimitiveTopology& GetPrimitiveTopology() const noexcept { return m_PrimitiveTopology; }

	protected:
		const std::shared_ptr<IShader> m_Shader;
		const std::vector<VertexAttrDescr>& m_InputLayout;

		PrimitiveTopology m_PrimitiveTopology;
	};

	IPSO::IPSO(const std::shared_ptr<IShader> _shader, const std::vector<VertexAttrDescr>& _inputLayout, PrimitiveTopology _topo) :
		m_Shader{ _shader },
		m_InputLayout{ _inputLayout },
		m_PrimitiveTopology{ _topo }
	{
		ensure(m_Shader != nullptr, ">>>>> [IPSO::IPSO( ... )]. Shader pointer cannot be null.");
		ensure(!m_InputLayout.empty(), ">>>>> [IPSO::IPSO( ... )]. Input layout cannot be empty.");
	}
}