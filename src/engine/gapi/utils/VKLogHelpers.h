#pragma once

#if defined(Z_VULKAN)
#include <vulkan/vulkan.h>
#include <format>
#include <string_view>

namespace zzz::engine::gapi
{
	constexpr std::string_view ToString(VkFormat format) noexcept
	{
		switch (format)
		{
		case VK_FORMAT_UNDEFINED:                  return "VkFormat::VK_FORMAT_UNDEFINED";
		case VK_FORMAT_R8G8B8A8_UNORM:             return "VkFormat::VK_FORMAT_R8G8B8A8_UNORM";
		case VK_FORMAT_R8G8B8A8_SRGB:              return "VkFormat::VK_FORMAT_R8G8B8A8_SRGB";
		case VK_FORMAT_B8G8R8A8_UNORM:             return "VkFormat::VK_FORMAT_B8G8R8A8_UNORM";
		case VK_FORMAT_B8G8R8A8_SRGB:              return "VkFormat::VK_FORMAT_B8G8R8A8_SRGB";
		case VK_FORMAT_D24_UNORM_S8_UINT:          return "VkFormat::VK_FORMAT_D24_UNORM_S8_UINT";
		case VK_FORMAT_D32_SFLOAT:                 return "VkFormat::VK_FORMAT_D32_SFLOAT";
		case VK_FORMAT_D32_SFLOAT_S8_UINT:         return "VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT";
		default: break;
		}
		return "VkFormat::UNKNOWN_VALUE";
	}
}

template <>
struct std::formatter<VkFormat> : std::formatter<std::string_view>
{
	auto format(VkFormat format, std::format_context& ctx) const
	{
		return std::formatter<std::string_view>::format(zzz::engine::gapi::ToString(format), ctx);
	}
};

#endif // Z_VULKAN
