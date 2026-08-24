#include "SurfView_DX.h"

#if defined(Z_D3D12)

namespace zzz::engine
{
	SurfView_DX::SurfView_DX(std::shared_ptr<NativeWindow> window, std::shared_ptr<IGAPI> gapi) :
		ISurfView(std::move(window), gapi),
		m_DirectX12API(std::dynamic_pointer_cast<DirectX12API>(gapi))
	{
		ensure(m_DirectX12API, "Failed to cast IGAPI to DirectX12API.");

		auto res = Initialize();
		if (!res)
		{
			THROW_RUNTIME("Failed to initialize SurfView_DX: {}", res.error());
		}
	}

	std::expected<void, std::string> SurfView_DX::Initialize()
	{
		return {};
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
