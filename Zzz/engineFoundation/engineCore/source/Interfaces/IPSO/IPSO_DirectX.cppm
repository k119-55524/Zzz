
#include "pch.h"

export module IPSO_DirectX;

export namespace zzz::core
{
	class IPSO_DirectX
	{
#if defined(ZRENDER_API_D3D12)
		Z_NO_COPY_MOVE(IPSO_DirectX);

	public:
		IPSO_DirectX() = default;
		virtual ~IPSO_DirectX() = default;

		virtual const ComPtr<ID3D12PipelineState> GetPSO()  const noexcept = 0;
#endif // defined(ZRENDER_API_D3D12)
	};
}