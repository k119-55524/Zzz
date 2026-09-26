#pragma once

#include <cstdint>
#include <string_view>
#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	/**
	 * @enum eShaderBinaryFormat
	 * @brief Формат скомпилированного бинарного байткода шейдера.
	 */
	enum class eShaderBinaryFormat : zU32
	{
		Unknown  = 0,
		DXIL     = 1, ///< Байткод для DirectX 12 (Windows)
		SPIRV    = 2, ///< Байткод для Vulkan (Windows / Linux / Android)
		MetalLib = 3  ///< Бинарный байткод для Metal (macOS / iOS)
	};

	constexpr std::string_view ToString(eShaderBinaryFormat format)
	{
		switch (format)
		{
		case eShaderBinaryFormat::Unknown:  return "Unknown";
		case eShaderBinaryFormat::DXIL:     return "DXIL";
		case eShaderBinaryFormat::SPIRV:    return "SPIRV";
		case eShaderBinaryFormat::MetalLib: return "MetalLib";
		}
		THROW_RUNTIME("Необработанный eShaderBinaryFormat");
	}
}
