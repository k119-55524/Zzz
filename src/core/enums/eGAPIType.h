#pragma once

#include <cstdint>
#include <string_view>
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	enum class eGAPIType : std::uint8_t
	{
		DirectX12,
		Vulkan,
		Metal
	};

	constexpr std::string_view ToString(eGAPIType type)
	{
		switch (type)
		{
		case eGAPIType::DirectX12: return "DirectX12";
		case eGAPIType::Vulkan:    return "Vulkan";
		case eGAPIType::Metal:     return "Metal";
		}
		THROW_RUNTIME("Необработанный eGAPIType");
	}
}
