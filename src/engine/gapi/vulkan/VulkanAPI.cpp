#include "engine/gapi/vulkan/VulkanAPI.h"

#if defined(Z_VULKAN)

namespace zzz::engine
{
	VulkanAPI::VulkanAPI(std::shared_ptr<UserSettingsManager> userSettings)
		: IGAPI(std::move(userSettings))
	{
	}

	VulkanAPI::~VulkanAPI()
	{
	}

	void VulkanAPI::Initialize()
	{
	}

	void VulkanAPI::SubmitCommandLists()
	{
	}

	void VulkanAPI::BeginRender()
	{
	}

	void VulkanAPI::EndRender()
	{
	}

	void VulkanAPI::WaitForGpu()
	{
	}
}

#endif // Z_VULKAN
