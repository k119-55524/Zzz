
export module PSO_DX;

#if defined(ZRENDER_API_D3D12)
import IPSO;
import IGAPI;
import DXAPI;
import PrimitiveTopology;
import VertexFormatMapper;

namespace zzz::dx
{
	export class PSO_DX final : public IPSO
	{
	public:
		explicit PSO_DX(
			const std::shared_ptr<IGAPI> m_GAPI,
			const std::shared_ptr<IShader> _shader,
			const std::vector<VertexAttrDescr>& _inputLayout,
			PrimitiveTopology _topo = PrimitiveTopology(eTopology::TriangleList));
		virtual ~PSO_DX() override = default;

		const ComPtr<ID3D12PipelineState> GetPSO() const noexcept { return m_PSO; };

	private:
		void CreatePSO(const std::shared_ptr<IGAPI> _IGAPI);
		D3D12_PRIMITIVE_TOPOLOGY_TYPE ConvertPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology);
		ComPtr<ID3D12PipelineState> m_PSO;
	};

	PSO_DX::PSO_DX(
		const std::shared_ptr<IGAPI> m_GAPI,
		const std::shared_ptr<IShader> _shader,
		const std::vector<VertexAttrDescr>& _inputLayout,
		PrimitiveTopology _topo) :
		IPSO(_shader, _inputLayout, _topo)
	{
		CreatePSO(m_GAPI);
	}

	void PSO_DX::CreatePSO(const std::shared_ptr<IGAPI> _IGAPI)
	{
		//D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		//{
		//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//	{ "COLOR",	  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		//};

		std::shared_ptr<DXAPI> dxAPI = std::dynamic_pointer_cast<DXAPI>(_IGAPI);
		ensure(dxAPI, "Failed to cast IGAPI to DXAPI.");

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		psoDesc.InputLayout = { m_InputLayout.data(), static_cast<UINT>(m_InputLayout.size()) };
		psoDesc.pRootSignature = dxAPI->GetRootSignature().Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_Shader->GetVS().Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_Shader->GetPS().Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

		// Настройки глубины и трафарета
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = TRUE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = ConvertPrimitiveTopology(m_PrimitiveTopology.ToD3D12());
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DEPTH_FORMAT;
		psoDesc.SampleDesc.Count = 1;
		HRESULT hr = dxAPI->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSO));
		if (hr != S_OK)
			throw_runtime_error(std::format("Failed to create pipeline state object. HRESULT = 0x{:08X}", hr));
	}

	D3D12_PRIMITIVE_TOPOLOGY_TYPE PSO_DX::ConvertPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology)
	{
		switch (topology)
		{
			// Point topologies
		case D3D_PRIMITIVE_TOPOLOGY_POINTLIST:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

			// Line topologies
		case D3D_PRIMITIVE_TOPOLOGY_LINELIST:
		case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
		case D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ:
		case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

			// Triangle topologies
		case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
		case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
		case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ:
		case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

			// Patch topologies (для tessellation)
		case D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST:
		case D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;

			// Undefined или неизвестные
		case D3D_PRIMITIVE_TOPOLOGY_UNDEFINED:
		default:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
		}
	}
}
#endif	// ZRENDER_API_D3D12