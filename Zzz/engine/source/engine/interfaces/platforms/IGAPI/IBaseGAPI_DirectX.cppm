
#include "pch.h"
export module IBaseGAPI_DirectX;

import result;

using namespace zzz;

export namespace zzz::platforms::directx
{
	export class IBaseGAPI_DirectX
	{
#if defined(ZRENDER_API_D3D12)
	public:
		IBaseGAPI_DirectX() = default;
		IBaseGAPI_DirectX(const IBaseGAPI_DirectX&) = delete;
		IBaseGAPI_DirectX(IBaseGAPI_DirectX&&) = delete;
		IBaseGAPI_DirectX& operator=(const IBaseGAPI_DirectX&) = delete;
		IBaseGAPI_DirectX& operator=(IBaseGAPI_DirectX&&) = delete;

		virtual const ComPtr<ID3D12Device> GetDevice() const noexcept = 0;
		virtual const ComPtr<ID3D12CommandQueue> GetCommandQueue() const noexcept = 0;
		virtual const ComPtr<IDXGIFactory7> GetFactory() const noexcept = 0;
		virtual const ComPtr<ID3D12GraphicsCommandList>& GetCommandListUpdate() const noexcept = 0;
		virtual const ComPtr<ID3D12GraphicsCommandList>& GetCommandListRender() const noexcept = 0;
		virtual ComPtr<ID3D12RootSignature> GetRootSignature() const noexcept = 0;

		virtual void CommandRenderReset() noexcept = 0;
		virtual [[nodiscard]] result<> CommandRenderReinitialize() = 0;
		virtual void EndPreparedTransfers() = 0;
	};
#endif // defined(ZRENDER_API_D3D12)
}