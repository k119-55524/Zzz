#include "SurfView_VK.h"

#if defined(Z_VULKAN)

namespace zzz::engine
{
	SurfView_VK::SurfView_VK(std::shared_ptr<NativeWindow> window, std::shared_ptr<IGAPI> gapi) :
		ISurfView(window, gapi),
		m_VulkanAPI(std::dynamic_pointer_cast<VulkanAPI>(gapi))
	{
		ensure(m_Window, "Window cannot be null.");
		ensure(m_VulkanAPI, "Failed to cast IGAPI to VulkanAPI.");

		auto res = Initialize();
		if (!res)
		{
			THROW_RUNTIME("Failed to initialize SurfView_VK: {}", res.error());
		}
	}

	std::expected<void, std::string> SurfView_VK::Initialize()
	{
		return {};
	}

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
