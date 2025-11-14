
#include "pch.h"

export module PSO_DX;

import IPSO;
import IGAPI;
import VertexFormatMapper;

using namespace zzz::core;

#if defined(ZRENDER_API_D3D12)
namespace zzz::directx
{
	export class PSO_DX final : public IPSO
	{
	public:
		PSO_DX() = delete;
		explicit PSO_DX(const std::shared_ptr<IGAPI> m_GAPI, const std::shared_ptr<IShader> _shader, const std::vector<VertexAttrDescr>& _inputLayout);
		virtual ~PSO_DX() override = default;

		const ComPtr<ID3D12PipelineState> GetPSO() const noexcept override { return m_PSO; };

	private:
		void CreatePSO(const std::shared_ptr<IGAPI> m_GAPI);

		ComPtr<ID3D12PipelineState> m_PSO;
	};

	PSO_DX::PSO_DX(const std::shared_ptr<IGAPI> m_GAPI, const std::shared_ptr<IShader> _shader, const std::vector<VertexAttrDescr>& _inputLayout) :
		IPSO(_shader, _inputLayout)
	{
		CreatePSO(m_GAPI);
	}

	void PSO_DX::CreatePSO(const std::shared_ptr<IGAPI> m_GAPI)
	{
		//D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		//{
		//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//	{ "COLOR",	  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		//};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		psoDesc.InputLayout = { m_InputLayout.data(), static_cast<UINT>(m_InputLayout.size()) };
		psoDesc.pRootSignature = m_GAPI->GetRootSignature().Get();
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
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DEPTH_FORMAT;
		psoDesc.SampleDesc.Count = 1;
		HRESULT hr = m_GAPI->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSO));
		if (hr != S_OK)
			throw_runtime_error(std::format(">>>>> [PSO_DX::CreatePSO()]. Failed to create pipeline state object. HRESULT = 0x{:08X}", hr));
	}
}
#endif	// ZRENDER_API_D3D12