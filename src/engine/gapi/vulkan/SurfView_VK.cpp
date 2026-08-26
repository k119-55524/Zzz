#include "SurfView_VK.h"

#if defined(Z_VULKAN)

namespace zzz::engine
{
	SurfView_VK::SurfView_VK(std::shared_ptr<NativeWindow> window, std::shared_ptr<VulkanAPI> gapi) :
		ISurfView(std::move(window), std::move(gapi))
	{
	}

#pragma region Init
	void SurfView_VK::Initialize()
	{
	}

	void SurfView_VK::OnSurfaceCreated(void* handle)
	{
		(void)handle;
		Initialize();
	}

	void SurfView_VK::OnSurfaceDestroyed()
	{
	}
#pragma endregion

	void SurfView_VK::PrepareFrame()
	{
	}

	void SurfView_VK::RenderFrame()
	{
	}

	void SurfView_VK::OnResize(const Size2D<>& size)
	{
	}
}

#endif // Z_VULKAN
