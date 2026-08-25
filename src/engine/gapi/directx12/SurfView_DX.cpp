
#include "SurfView_DX.h"

#if defined(Z_D3D12)
namespace zzz::engine
{
	SurfView_DX::SurfView_DX(std::shared_ptr<NativeWindow> window, std::shared_ptr<DirectX12API> gapi)
		: ISurfView(std::move(window), std::move(gapi))
	{
		Initialize();
	}

	SurfView_DX::~SurfView_DX()
	{
		m_GAPI->WaitForGpu();

		if (m_DepthBuffer)
			m_DepthBuffer->Release();

		if (m_Swapchain)
			m_Swapchain->Release();
	}

	void SurfView_DX::Initialize()
	{
		m_Swapchain = std::make_unique<Swapchain_DX>(m_GAPI, m_Window);
		m_OldSize = m_Swapchain->GetSize();
		m_DepthBuffer = std::make_unique<DepthBuffer_DX>(m_GAPI, m_OldSize);
	}

	void SurfView_DX::PrepareFrame()
	{
	}

	void SurfView_DX::RenderFrame()
	{
		if (m_Swapchain)
			m_Swapchain->Present(true);
	}

	void SurfView_DX::OnResize(const Size2D<>& size)
	{
		if (!m_Swapchain || !m_DepthBuffer)
			return;

		if (size.GetWidth() == 0 || size.GetHeight() == 0)
		{
			DOut("[SurfView_DX::OnResize] Width or height is zero.");
			return;
		}

		if (m_OldSize == size)
		{
			DOut("[SurfView_DX::OnResize] Dimensions are unchanged ({}x{}).", size.GetWidth(), size.GetHeight());
			return;
		}

		m_GAPI->WaitForGpu();
		m_Swapchain->OnResize(size);
		m_DepthBuffer->OnResize(size);

		m_OldSize = size;
		DOut("[SurfView_DX::OnResize] Resize completed from old size to {}x{}.", size.GetWidth(), size.GetHeight());
	}
}
#endif // Z_D3D12
