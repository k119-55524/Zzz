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
		void WaitForGpu() override;

	private:
		friend class Engine;
		void Initialize() override;
	};
}

#endif // Z_VULKAN
