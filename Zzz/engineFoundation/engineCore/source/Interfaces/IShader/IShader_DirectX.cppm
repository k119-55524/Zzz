
#include "pch.h"
export module IShader_DirectX;

export namespace zzz
{
	export class IShader_DirectX
	{
#if defined(ZRENDER_API_D3D12)
		Z_NO_COPY_MOVE(IShader_DirectX);

	public:
		IShader_DirectX() = default;
		virtual ~IShader_DirectX() = default;

		virtual ComPtr<ID3DBlob> GetVS() const noexcept = 0;
		virtual ComPtr<ID3DBlob> GetPS() const noexcept = 0;
#endif // defined(ZRENDER_API_D3D12)
	};
}