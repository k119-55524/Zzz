#pragma once

#include "engine/gapi/IGAPI.h"

#if defined(Z_VULKAN)

namespace zzz::engine
{
	class VulkanAPI final : public IGAPI
	{
	public:
		explicit VulkanAPI(std::shared_ptr<UserSettingsManager> userSettings);
		~VulkanAPI() override;

		void SubmitCommandLists() override;
		void BeginRender() override;
		void EndRender() override;

	protected:
		[[nodiscard]] std::expected<void, std::string> Init() override;
		void WaitForGpu() override;
	};
}

#endif // Z_VULKAN
