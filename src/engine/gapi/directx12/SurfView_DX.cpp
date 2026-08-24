#include "SurfView_DX.h"

#if defined(Z_D3D12)

namespace zzz::engine
{
	SurfView_DX::SurfView_DX(std::shared_ptr<NativeWindow> window, std::shared_ptr<DirectX12API> gapi) :
		ISurfView(std::move(window), std::move(gapi))
	{
		Initialize();
	}

	void SurfView_DX::Initialize()
	{
	}

	void SurfView_DX::PrepareFrame()
	{
	}

	void SurfView_DX::RenderFrame()
	{
	}

	void SurfView_DX::OnResize(const Size2D<>& size)
	{
	}
}

#endif // Z_D3D12
