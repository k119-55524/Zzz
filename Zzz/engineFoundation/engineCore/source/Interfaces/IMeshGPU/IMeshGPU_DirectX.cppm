
#include "pch.h"

export module IMeshGPU_DirectX;

export namespace zzz::engineCore
{
	export class IMeshGPU_DirectX
	{
#if defined(ZRENDER_API_D3D12)
		Z_NO_COPY_MOVE(IMeshGPU_DirectX);

	public:
		IMeshGPU_DirectX() = default;
		virtual ~IMeshGPU_DirectX() = default;

		virtual const D3D12_VERTEX_BUFFER_VIEW* VertexBufferView() const = 0;
		virtual const D3D12_INDEX_BUFFER_VIEW* IndexBufferView() const = 0;
#endif // defined(ZRENDER_API_D3D12)
	};
}