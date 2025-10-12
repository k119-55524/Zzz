
#include "pch.h"
export module IBaseMeshGPU_DirectX;

export namespace zzz::platforms::directx
{
	export class IBaseMeshGPU_DirectX
	{
#if defined(ZRENDER_API_D3D12)
	public:
		IBaseMeshGPU_DirectX() = default;
		IBaseMeshGPU_DirectX(const IBaseMeshGPU_DirectX&) = delete;
		IBaseMeshGPU_DirectX(IBaseMeshGPU_DirectX&&) = delete;
		IBaseMeshGPU_DirectX& operator=(const IBaseMeshGPU_DirectX&) = delete;
		IBaseMeshGPU_DirectX& operator=(IBaseMeshGPU_DirectX&&) = delete;

		virtual ~IBaseMeshGPU_DirectX() = default;

		virtual const D3D12_VERTEX_BUFFER_VIEW* VertexBufferView() const = 0;
		virtual const D3D12_INDEX_BUFFER_VIEW* IndexBufferView() const = 0;
#endif // defined(ZRENDER_API_D3D12)
	};
}