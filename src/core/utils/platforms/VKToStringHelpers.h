#pragma once

#if defined(Z_VULKAN)
#include <vulkan/vulkan.h>
#include <format>
#include <string_view>

namespace zzz::core
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

	constexpr std::string_view ToString(VkResult result) noexcept
	{
		switch (result)
		{
		case VK_SUCCESS:                        return "VK_SUCCESS";
		case VK_NOT_READY:                      return "VK_NOT_READY";
		case VK_TIMEOUT:                        return "VK_TIMEOUT";
		case VK_EVENT_SET:                      return "VK_EVENT_SET";
		case VK_EVENT_RESET:                    return "VK_EVENT_RESET";
		case VK_INCOMPLETE:                     return "VK_INCOMPLETE";
		case VK_ERROR_OUT_OF_HOST_MEMORY:       return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:     return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case VK_ERROR_INITIALIZATION_FAILED:    return "VK_ERROR_INITIALIZATION_FAILED";
		case VK_ERROR_DEVICE_LOST:              return "VK_ERROR_DEVICE_LOST";
		case VK_ERROR_MEMORY_MAP_FAILED:        return "VK_ERROR_MEMORY_MAP_FAILED";
		case VK_ERROR_LAYER_NOT_PRESENT:        return "VK_ERROR_LAYER_NOT_PRESENT";
		case VK_ERROR_EXTENSION_NOT_PRESENT:    return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case VK_ERROR_FEATURE_NOT_PRESENT:      return "VK_ERROR_FEATURE_NOT_PRESENT";
		case VK_ERROR_INCOMPATIBLE_DRIVER:      return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case VK_ERROR_TOO_MANY_OBJECTS:         return "VK_ERROR_TOO_MANY_OBJECTS";
		case VK_ERROR_FORMAT_NOT_SUPPORTED:     return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		case VK_ERROR_FRAGMENTED_POOL:          return "VK_ERROR_FRAGMENTED_POOL";
		case VK_ERROR_UNKNOWN:                  return "VK_ERROR_UNKNOWN";
		case VK_ERROR_OUT_OF_DATE_KHR:          return "VK_ERROR_OUT_OF_DATE_KHR";
		case VK_SUBOPTIMAL_KHR:                 return "VK_SUBOPTIMAL_KHR";
		case VK_ERROR_SURFACE_LOST_KHR:         return "VK_ERROR_SURFACE_LOST_KHR";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		default: break;
		}
		return "VkResult::UNKNOWN_VALUE";
	}

	constexpr std::string_view ToString(VkPresentModeKHR mode) noexcept
	{
		switch (mode)
		{
		case VK_PRESENT_MODE_IMMEDIATE_KHR:                 return "VK_PRESENT_MODE_IMMEDIATE_KHR";
		case VK_PRESENT_MODE_MAILBOX_KHR:                   return "VK_PRESENT_MODE_MAILBOX_KHR";
		case VK_PRESENT_MODE_FIFO_KHR:                      return "VK_PRESENT_MODE_FIFO_KHR";
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:              return "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
		case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:     return "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR";
		case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR: return "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR";
		default: break;
		}
		return "VkPresentModeKHR::UNKNOWN_VALUE";
	}
}

template <>
struct std::formatter<VkResult> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return std::formatter<std::string_view>::parse(ctx);
	}

	auto format(VkResult result, std::format_context& ctx) const
	{
		return std::formatter<std::string_view>::format(zzz::core::ToString(result), ctx);
	}
};

template <>
struct std::formatter<VkPresentModeKHR> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return std::formatter<std::string_view>::parse(ctx);
	}

	auto format(VkPresentModeKHR mode, std::format_context& ctx) const
	{
		return std::formatter<std::string_view>::format(zzz::core::ToString(mode), ctx);
	}
};

template <>
struct std::formatter<VkFormat> : std::formatter<std::string_view>
{
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return std::formatter<std::string_view>::parse(ctx);
	}

	auto format(VkFormat format, std::format_context& ctx) const
	{
		return std::formatter<std::string_view>::format(zzz::core::ToString(format), ctx);
	}
};

#endif // Z_VULKAN
