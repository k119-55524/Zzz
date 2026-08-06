#include "engine/gapi/vulkan/VulkanAPI.h"

#if defined(Z_VULKAN)

namespace zzz::engine
{
	VulkanAPI::VulkanAPI(std::shared_ptr<UserSettingsManager> userSettings)
		: IGAPI(std::move(userSettings), eGAPIType::Vulkan)
	{
	}

	VulkanAPI::~VulkanAPI()
	{
	}

	std::expected<void, std::string> VulkanAPI::Init()
	{
		return {};
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
