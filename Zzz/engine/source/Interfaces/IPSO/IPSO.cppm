
export module IPSO;

import IGAPI;
import Ensure;
import IShader;
import PrimitiveTopology;
import VertexFormatMapper;

//#if defined(ZRENDER_API_D3D12)
////using namespace zzz::dx;
//#elif defined(ZRENDER_API_VULKAN)
////using namespace zzz::vk;
//#else
//#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
//#endif

namespace zzz
{
	export class IPSO
	{
		Z_NO_COPY_MOVE(IPSO);

	public:
		IPSO() = delete;
		explicit IPSO(const std::shared_ptr<IShader>& shader, const std::vector<VertexAttrDescr>& inputLayout, const PrimitiveTopology& topo);
		virtual ~IPSO() {};

		const PrimitiveTopology& GetPrimitiveTopology() const noexcept { return m_PrimitiveTopology; }

	protected:
		std::shared_ptr<IShader> m_Shader;
		std::vector<VertexAttrDescr> m_InputLayout;

		PrimitiveTopology m_PrimitiveTopology;
	};

	IPSO::IPSO(const std::shared_ptr<IShader>& shader, const std::vector<VertexAttrDescr>& inputLayout, const PrimitiveTopology& topo) :
		m_Shader{ shader },
		m_InputLayout{ inputLayout },
		m_PrimitiveTopology{ topo }
	{
		ensure(m_Shader != nullptr, "Shader pointer cannot be null.");
		ensure(!m_InputLayout.empty(), "Input layout cannot be empty.");
	}
}