
#include "pch.h"
export module IBaseShader_DirectX;

export namespace zzz::platforms::directx
{
	export class IBaseShader
	{
#if defined(ZRENDER_API_D3D12)
	public:
		IBaseShader() = default;
		IBaseShader(const IBaseShader&) = delete;
		IBaseShader(IBaseShader&&) = delete;
		IBaseShader& operator=(const IBaseShader&) = delete;
		IBaseShader& operator=(IBaseShader&&) = delete;

		virtual ~IBaseShader() = default;

		virtual ComPtr<ID3DBlob> GetVS() const noexcept = 0;
		virtual ComPtr<ID3DBlob> GetPS() const noexcept = 0;
#endif // defined(ZRENDER_API_D3D12)
	};
}