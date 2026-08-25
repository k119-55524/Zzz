#pragma once

//#include "core/Core.h"
#include "engine/gapi/directx12/DirectX12API.h"

#if defined(Z_D3D12)
namespace zzz::engine
{
	class DepthBuffer_DX final
	{
		Z_NO_COPY_MOVE(DepthBuffer_DX);

	public:
		DepthBuffer_DX(std::shared_ptr<DirectX12API> gapi, const Size2D<>& size);
		~DepthBuffer_DX();

		void OnResize(const Size2D<>& size);
		void Release();

		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle() const noexcept { return m_DsvHandle; }
		[[nodiscard]] ID3D12Resource* GetResource() const noexcept { return m_DepthStencilBuffer.Get(); }

	private:
		void Initialize(const Size2D<>& size);

		std::shared_ptr<DirectX12API> m_GAPI;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE m_DsvHandle;
	};
}
#endif // Z_D3D12
