#pragma once

#include <cstdint>

namespace zzz::core
{
	enum class eGAPIType : std::uint8_t
	{
		DirectX12,
		Vulkan,
		Metal
	};
}
