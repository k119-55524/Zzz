
#include "pch.h"
export module IBaseGAPI_DirectX;

import result;

using namespace zzz;

export namespace zzz::core
{
	export class IGAPI_DirectX
	{
#if defined(ZRENDER_API_D3D12)
		Z_NO_COPY_MOVE(IGAPI_DirectX);

	public:
		IGAPI_DirectX() = default;
		virtual ~IGAPI_DirectX() = default;

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