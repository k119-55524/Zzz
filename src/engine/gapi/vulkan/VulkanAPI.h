#pragma once

#include "engine/gapi/IGAPI.h"
#include "engine/gapi/selectors/gpu/vulkan/VulkanGpuSelector.h"

#if defined(Z_VULKAN)

namespace zzz::engine
{
	class VulkanAPI final : public IGAPI
	{
	public:
		explicit VulkanAPI() = default;
		~VulkanAPI() override;

		void WaitForGpu() override;

		[[nodiscard]] VkInstance GetInstance() const noexcept { return m_Instance; }
		[[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const noexcept { return m_PhysicalDevice; }
		[[nodiscard]] VkDevice GetDevice() const noexcept { return m_Device; }
		[[nodiscard]] VkQueue GetGraphicsQueue() const noexcept { return m_GraphicsQueue; }
		[[nodiscard]] VkQueue GetPresentQueue() const noexcept { return m_PresentQueue; }
		[[nodiscard]] uint32_t GetGraphicsQueueFamilyIndex() const noexcept { return m_GraphicsQueueFamilyIndex; }
		[[nodiscard]] uint32_t GetPresentQueueFamilyIndex() const noexcept { return m_PresentQueueFamilyIndex; }

	private:
		friend class Engine;
		void Initialize(std::shared_ptr<UserSettingsManager> userSettings) override;

		void CreateInstance();
		void EnableDebugMessenger();
		void SelectPhysicalDeviceAndCreateLogicalDevice(std::shared_ptr<UserSettingsManager> userSettings);

		VkInstance m_Instance{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT m_DebugMessenger{ VK_NULL_HANDLE };
		VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
		VkDevice m_Device{ VK_NULL_HANDLE };
		VkQueue m_GraphicsQueue{ VK_NULL_HANDLE };
		VkQueue m_PresentQueue{ VK_NULL_HANDLE };

		uint32_t m_GraphicsQueueFamilyIndex{ UINT32_MAX };
		uint32_t m_PresentQueueFamilyIndex{ UINT32_MAX };

		VkFence m_RenderFence{ VK_NULL_HANDLE };
	};
}

#endif // Z_VULKAN
