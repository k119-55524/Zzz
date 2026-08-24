#pragma once

#include "engine/gapi/directx12/DirectX12API.h"

#if defined(Z_D3D12)

namespace zzz::dx12
{
	class DepthBuffer_DX final
	{
		Z_NO_COPY_MOVE(DepthBuffer_DX);

	public:
		DepthBuffer_DX() = default;
		~DepthBuffer_DX();

		std::expected<void, std::string> Initialize(std::shared_ptr<zzz::engine::DirectX12API> gapi, const Size2D<>& size);
		void Release();

		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle() const noexcept { return m_DsvHandle; }
		[[nodiscard]] ID3D12Resource* GetResource() const noexcept { return m_DepthStencilBuffer.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE m_DsvHandle{};
		DXGI_FORMAT m_DepthFormat{ DXGI_FORMAT_D24_UNORM_S8_UINT };
	};
}

#endif // Z_D3D12
