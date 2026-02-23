
export module PSO_VK;

#if defined(ZRENDER_API_VULKAN)

import IPSO;
import IGAPI;
import IShader;
import PrimitiveTopology;
import VertexFormatMapper;

namespace zzz::vk
{
	export class PSO_VK final : public IPSO
	{
	public:
		explicit PSO_VK(
			const std::shared_ptr<IGAPI>& m_GAPI,
			const std::shared_ptr<IShader>& shader,
			const std::vector<VertexAttrDescr>& inputLayout,
			PrimitiveTopology _topo = PrimitiveTopology(eTopology::TriangleList));

		virtual ~PSO_VK() override = default;
	};

	PSO_VK::PSO_VK(
		const std::shared_ptr<IGAPI>& m_GAPI,
		const std::shared_ptr<IShader>& shader,
		const std::vector<VertexAttrDescr>& inputLayout,
		PrimitiveTopology topo) :
		IPSO(shader, inputLayout, topo)
	{
		//CreatePSO(m_GAPI);
	}
}
#endif // ZRENDER_API_VULKAN