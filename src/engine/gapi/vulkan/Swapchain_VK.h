#pragma once

#include "engine/gapi/vulkan/VulkanAPI.h"
#include "engine/gapi/clear_config/SurfaceClearConfig.h"
#include "engine/platforms/window/NativeWindow.h"

#if defined(Z_VULKAN)

namespace zzz::engine
{
	using namespace zzz::core;

	class Swapchain_VK
	{
		Z_NO_COPY_MOVE(Swapchain_VK);

	public:
		Swapchain_VK(std::shared_ptr<VulkanAPI> gapi, std::shared_ptr<NativeWindow> window);
		~Swapchain_VK();

		void Initialize(VkSurfaceKHR surface);
		void Release();

		void Present(bool vSync, uint32_t imageIndex, VkSemaphore waitSemaphore);
		void OnResize(const Size2D<>& size);

		void SetClearConfig(const SurfaceClearConfig& config) noexcept { m_ClearConfig = config; }
		[[nodiscard]] const SurfaceClearConfig& GetClearConfig() const noexcept { return m_ClearConfig; }

		[[nodiscard]] VkSwapchainKHR GetSwapchain() const noexcept { return m_Swapchain; }
		[[nodiscard]] VkSurfaceKHR GetSurface() const noexcept { return m_Surface; }
		[[nodiscard]] const Size2D<>& GetSize() const noexcept { return m_Size; }
		[[nodiscard]] VkFormat GetFormat() const noexcept { return m_Format; }
		[[nodiscard]] size_t GetImageCount() const noexcept { return m_Images.size(); }
		[[nodiscard]] VkImage GetBackBuffer(uint32_t imageIndex) const noexcept;
		[[nodiscard]] VkImageView GetImageView(uint32_t imageIndex) const noexcept;

	private:
		void CreateSwapchain(const Size2D<>& size);
		void CleanupSwapchain();

		std::shared_ptr<VulkanAPI> m_GAPI;
		std::shared_ptr<NativeWindow> m_Window;

		VkSurfaceKHR m_Surface{ VK_NULL_HANDLE };
		VkSwapchainKHR m_Swapchain{ VK_NULL_HANDLE };
		VkFormat m_Format{ c_DefaultBackBufferFormat };
		Size2D<> m_Size{};

		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;

		SurfaceClearConfig m_ClearConfig;
		uint32_t m_FrameIndex{ 0 };
	};
}

#endif // Z_VULKAN
