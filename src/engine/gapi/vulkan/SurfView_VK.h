#pragma once

#include "engine/gapi/ISurfView.h"
#include "engine/gapi/vulkan/VulkanAPI.h"
#include "engine/platforms/window/NativeWindow.h"

#if defined(Z_VULKAN)
namespace zzz::engine
{
	class SurfView_VK final : public ISurfView
	{
		Z_NO_COPY_MOVE(SurfView_VK);

	public:
		SurfView_VK(std::shared_ptr<NativeWindow> window, std::shared_ptr<VulkanAPI> gapi);
		~SurfView_VK() override = default;

		void PrepareFrame() override;
		void RenderFrame() override;
		void OnResize(const Size2D<>& size) override;

	protected:
		void Initialize() override;
	};
}
#endif // Z_VULKAN
