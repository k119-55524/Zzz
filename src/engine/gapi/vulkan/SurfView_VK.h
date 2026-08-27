#pragma once

#include <mutex>
#include <array>
#include "engine/gapi/ISurfView.h"
#include "engine/gapi/vulkan/VulkanAPI.h"
#include "engine/platforms/window/NativeWindow.h"

#if defined(Z_VULKAN)

using namespace zzz::core;

namespace zzz::engine
{
	class SurfView_VK final : public ISurfView
	{
		Z_NO_COPY_MOVE(SurfView_VK);

	public:
		SurfView_VK(std::shared_ptr<NativeWindow> window, std::shared_ptr<VulkanAPI> gapi);
		~SurfView_VK() override;

		void PreRender() override;
		void PrepareFrame() override;
		void RenderFrame() override;
		void OnResize(const Size2D<>& size) override;

#pragma region Surface Lifecycle
		void OnSurfaceCreated(void* handle) override;
		void OnSurfaceDestroyed() override;
#pragma endregion

	protected:
		void Initialize() override;

	private:
		VkSurfaceKHR CreateVulkanSurface(void* handle);

		VkCommandPool m_CommandPool{ VK_NULL_HANDLE };
		std::array<VkCommandBuffer, c_FramesInFlight> m_CommandBuffers{ VK_NULL_HANDLE, VK_NULL_HANDLE };
		std::array<VkSemaphore, c_FramesInFlight> m_ImageAvailableSemaphores{ VK_NULL_HANDLE, VK_NULL_HANDLE };
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::array<VkFence, c_FramesInFlight> m_InFlightFences{ VK_NULL_HANDLE, VK_NULL_HANDLE };

		std::array<bool, c_FramesInFlight> m_IsRecording{};
		std::array<bool, c_FramesInFlight> m_FrameReady{};
		std::array<uint32_t, c_FramesInFlight> m_CurrentImageIndex{};
		bool m_IsDepthInitialLayoutTransitioned{ false };
		std::mutex m_SubmitMutex;
	};
}

#endif // Z_VULKAN
