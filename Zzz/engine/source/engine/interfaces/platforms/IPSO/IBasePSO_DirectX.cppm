
#include "pch.h"
export module IBasePSO_DirectX;

export namespace zzz::platforms::directx
{
	class IBasePSO_DirectX
	{
#if defined(ZRENDER_API_D3D12)
	public:
		IBasePSO_DirectX() = default;
		IBasePSO_DirectX(const IBasePSO_DirectX&) = delete;
		IBasePSO_DirectX(IBasePSO_DirectX&&) = delete;
		IBasePSO_DirectX& operator=(const IBasePSO_DirectX&) = delete;
		IBasePSO_DirectX& operator=(IBasePSO_DirectX&&) = delete;

		virtual ~IBasePSO_DirectX() = default;

		virtual const ComPtr<ID3D12PipelineState> GetPSO()  const noexcept = 0;
#endif // defined(ZRENDER_API_D3D12)
	};
}