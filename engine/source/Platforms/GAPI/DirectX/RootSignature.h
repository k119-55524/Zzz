#pragma once

#include "../IGAPI.h"

namespace Zzz::Platforms
{
#ifdef _GAPI_DX12

	class DirectX12API;

	class RootSignature
	{
	public:
		RootSignature();
		RootSignature(RootSignature&) = delete;
		RootSignature(RootSignature&&) = delete;

		const ComPtr<ID3D12RootSignature> Get() const noexcept { return m_RootSignature; };
		HRESULT Initialize(ComPtr<ID3D12Device> device);

	private:
		ComPtr<ID3D12RootSignature> m_RootSignature;
		std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
	};

#endif // _GAPI_DX12
}