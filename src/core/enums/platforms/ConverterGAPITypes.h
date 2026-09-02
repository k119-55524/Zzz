#pragma once

#include "core/utils/Defines.h"

#include <type_traits>
#include "core/enums/ePixelFormat.h"
#include "core/enums/eIndexFormat.h"

#if defined(Z_D3D12)
	#include <dxgiformat.h>
#elif defined(Z_VULKAN)
	#include <vulkan/vulkan.h>
#elif defined(Z_METAL)
	#import <Metal/Metal.h>
#endif

namespace zzz::core
{
#if defined(Z_D3D12)
	using NativePixelFormat = DXGI_FORMAT;
	using NativeIndexFormat = DXGI_FORMAT;
#elif defined(Z_VULKAN)
	using NativePixelFormat = VkFormat;
	using NativeIndexFormat = VkIndexType;
#elif defined(Z_METAL)
	using NativePixelFormat = MTLPixelFormat;
	using NativeIndexFormat = MTLIndexType;
#else
	using NativePixelFormat = zU32;
	using NativeIndexFormat = zU32;
#endif

	/**
	 * @class ConverterGAPITypes
	 * @brief Двусторонний конвертер между кроссплатформенными форматами движка и типами активного GAPI.
	 */
	class ConverterGAPITypes final
	{
	public:
		ConverterGAPITypes() = delete;

		[[nodiscard]] static constexpr NativePixelFormat ToNative(ePixelFormat format) noexcept
		{
#if defined(Z_D3D12)
			switch (format)
			{
			case ePixelFormat::R8_UNORM:          return DXGI_FORMAT_R8_UNORM;
			case ePixelFormat::RGBA8_UNORM:       return DXGI_FORMAT_R8G8B8A8_UNORM;
			case ePixelFormat::RGBA8_SRGB:        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			case ePixelFormat::BGRA8_UNORM:       return DXGI_FORMAT_B8G8R8A8_UNORM;
			case ePixelFormat::BGRA8_SRGB:        return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
			case ePixelFormat::RGBA16_FLOAT:      return DXGI_FORMAT_R16G16B16A16_FLOAT;
			case ePixelFormat::R32_FLOAT:         return DXGI_FORMAT_R32_FLOAT;
			case ePixelFormat::D32_FLOAT:         return DXGI_FORMAT_D32_FLOAT;
			case ePixelFormat::D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
			case ePixelFormat::D32_FLOAT_S8_UINT: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			case ePixelFormat::BC1_UNORM:         return DXGI_FORMAT_BC1_UNORM;
			case ePixelFormat::BC3_UNORM:         return DXGI_FORMAT_BC3_UNORM;
			case ePixelFormat::BC7_UNORM:         return DXGI_FORMAT_BC7_UNORM;
			default:                              return DXGI_FORMAT_UNKNOWN;
			}
#elif defined(Z_VULKAN)
			switch (format)
			{
			case ePixelFormat::R8_UNORM:          return VK_FORMAT_R8_UNORM;
			case ePixelFormat::RGBA8_UNORM:       return VK_FORMAT_R8G8B8A8_UNORM;
			case ePixelFormat::RGBA8_SRGB:        return VK_FORMAT_R8G8B8A8_SRGB;
			case ePixelFormat::BGRA8_UNORM:       return VK_FORMAT_B8G8R8A8_UNORM;
			case ePixelFormat::BGRA8_SRGB:        return VK_FORMAT_B8G8R8A8_SRGB;
			case ePixelFormat::RGBA16_FLOAT:      return VK_FORMAT_R16G16B16A16_SFLOAT;
			case ePixelFormat::R32_FLOAT:         return VK_FORMAT_R32_SFLOAT;
			case ePixelFormat::D32_FLOAT:         return VK_FORMAT_D32_SFLOAT;
			case ePixelFormat::D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
			case ePixelFormat::D32_FLOAT_S8_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
			case ePixelFormat::BC1_UNORM:         return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
			case ePixelFormat::BC3_UNORM:         return VK_FORMAT_BC3_UNORM_BLOCK;
			case ePixelFormat::BC7_UNORM:         return VK_FORMAT_BC7_UNORM_BLOCK;
			case ePixelFormat::ASTC_4x4_UNORM:    return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
			case ePixelFormat::ETC2_RGBA8_UNORM:  return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
			default:                              return VK_FORMAT_UNDEFINED;
			}
#elif defined(Z_METAL)
			switch (format)
			{
			case ePixelFormat::R8_UNORM:          return MTLPixelFormatR8Unorm;
			case ePixelFormat::RGBA8_UNORM:       return MTLPixelFormatRGBA8Unorm;
			case ePixelFormat::RGBA8_SRGB:        return MTLPixelFormatRGBA8Unorm_sRGB;
			case ePixelFormat::BGRA8_UNORM:       return MTLPixelFormatBGRA8Unorm;
			case ePixelFormat::BGRA8_SRGB:        return MTLPixelFormatBGRA8Unorm_sRGB;
			case ePixelFormat::RGBA16_FLOAT:      return MTLPixelFormatRGBA16Float;
			case ePixelFormat::R32_FLOAT:         return MTLPixelFormatR32Float;
			case ePixelFormat::D32_FLOAT:         return MTLPixelFormatDepth32Float;
			case ePixelFormat::D24_UNORM_S8_UINT: return MTLPixelFormatDepth24Unorm_Stencil8;
			case ePixelFormat::D32_FLOAT_S8_UINT: return MTLPixelFormatDepth32Float_Stencil8;
			case ePixelFormat::BC1_UNORM:         return MTLPixelFormatBC1_RGBA;
			case ePixelFormat::BC7_UNORM:         return MTLPixelFormatBC7_RGBAUnorm;
			case ePixelFormat::ASTC_4x4_UNORM:    return MTLPixelFormatASTC_4x4_LDR;
			default:                              return MTLPixelFormatInvalid;
			}
#else
			return static_cast<NativePixelFormat>(format);
#endif
		}

		// --- eIndexFormat -> NativeIndexFormat ---

		[[nodiscard]] static constexpr NativeIndexFormat ToNative(eIndexFormat format) noexcept
		{
#if defined(Z_D3D12)
			return (format == eIndexFormat::UInt16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
#elif defined(Z_VULKAN)
			return (format == eIndexFormat::UInt16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
#elif defined(Z_METAL)
			return (format == eIndexFormat::UInt16) ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32;
#else
			return static_cast<NativeIndexFormat>(format);
#endif
		}

		// --- NativePixelFormat -> ePixelFormat ---

		[[nodiscard]] static constexpr ePixelFormat ToEnginePixelFormat(NativePixelFormat nativeFormat) noexcept
		{
#if defined(Z_D3D12)
			switch (nativeFormat)
			{
			case DXGI_FORMAT_R8_UNORM:            return ePixelFormat::R8_UNORM;
			case DXGI_FORMAT_R8G8B8A8_UNORM:      return ePixelFormat::RGBA8_UNORM;
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return ePixelFormat::RGBA8_SRGB;
			case DXGI_FORMAT_B8G8R8A8_UNORM:      return ePixelFormat::BGRA8_UNORM;
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return ePixelFormat::BGRA8_SRGB;
			case DXGI_FORMAT_R16G16B16A16_FLOAT:  return ePixelFormat::RGBA16_FLOAT;
			case DXGI_FORMAT_R32_FLOAT:           return ePixelFormat::R32_FLOAT;
			case DXGI_FORMAT_D32_FLOAT:           return ePixelFormat::D32_FLOAT;
			case DXGI_FORMAT_D24_UNORM_S8_UINT:   return ePixelFormat::D24_UNORM_S8_UINT;
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return ePixelFormat::D32_FLOAT_S8_UINT;
			case DXGI_FORMAT_BC1_UNORM:           return ePixelFormat::BC1_UNORM;
			case DXGI_FORMAT_BC3_UNORM:           return ePixelFormat::BC3_UNORM;
			case DXGI_FORMAT_BC7_UNORM:           return ePixelFormat::BC7_UNORM;
			default:                              return ePixelFormat::Unknown;
			}
#elif defined(Z_VULKAN)
			switch (nativeFormat)
			{
			case VK_FORMAT_R8_UNORM:                  return ePixelFormat::R8_UNORM;
			case VK_FORMAT_R8G8B8A8_UNORM:            return ePixelFormat::RGBA8_UNORM;
			case VK_FORMAT_R8G8B8A8_SRGB:             return ePixelFormat::RGBA8_SRGB;
			case VK_FORMAT_B8G8R8A8_UNORM:            return ePixelFormat::BGRA8_UNORM;
			case VK_FORMAT_B8G8R8A8_SRGB:             return ePixelFormat::BGRA8_SRGB;
			case VK_FORMAT_R16G16B16A16_SFLOAT:       return ePixelFormat::RGBA16_FLOAT;
			case VK_FORMAT_R32_SFLOAT:                return ePixelFormat::R32_FLOAT;
			case VK_FORMAT_D32_SFLOAT:                return ePixelFormat::D32_FLOAT;
			case VK_FORMAT_D24_UNORM_S8_UINT:         return ePixelFormat::D24_UNORM_S8_UINT;
			case VK_FORMAT_D32_SFLOAT_S8_UINT:        return ePixelFormat::D32_FLOAT_S8_UINT;
			case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:      return ePixelFormat::BC1_UNORM;
			case VK_FORMAT_BC3_UNORM_BLOCK:           return ePixelFormat::BC3_UNORM;
			case VK_FORMAT_BC7_UNORM_BLOCK:           return ePixelFormat::BC7_UNORM;
			case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:      return ePixelFormat::ASTC_4x4_UNORM;
			case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK: return ePixelFormat::ETC2_RGBA8_UNORM;
			default:                                  return ePixelFormat::Unknown;
			}
#elif defined(Z_METAL)
			switch (nativeFormat)
			{
			case MTLPixelFormatR8Unorm:               return ePixelFormat::R8_UNORM;
			case MTLPixelFormatRGBA8Unorm:            return ePixelFormat::RGBA8_UNORM;
			case MTLPixelFormatRGBA8Unorm_sRGB:       return ePixelFormat::RGBA8_SRGB;
			case MTLPixelFormatBGRA8Unorm:            return ePixelFormat::BGRA8_UNORM;
			case MTLPixelFormatBGRA8Unorm_sRGB:       return ePixelFormat::BGRA8_SRGB;
			case MTLPixelFormatRGBA16Float:           return ePixelFormat::RGBA16_FLOAT;
			case MTLPixelFormatR32Float:              return ePixelFormat::R32_FLOAT;
			case MTLPixelFormatDepth32Float:          return ePixelFormat::D32_FLOAT;
			case MTLPixelFormatDepth24Unorm_Stencil8: return ePixelFormat::D24_UNORM_S8_UINT;
			case MTLPixelFormatDepth32Float_Stencil8: return ePixelFormat::D32_FLOAT_S8_UINT;
			case MTLPixelFormatBC1_RGBA:              return ePixelFormat::BC1_UNORM;
			case MTLPixelFormatBC7_RGBAUnorm:         return ePixelFormat::BC7_UNORM;
			case MTLPixelFormatASTC_4x4_LDR:          return ePixelFormat::ASTC_4x4_UNORM;
			default:                              return ePixelFormat::Unknown;
			}
#else
			return static_cast<ePixelFormat>(nativeFormat);
#endif
		}

		// --- NativeIndexFormat -> eIndexFormat ---

		[[nodiscard]] static constexpr eIndexFormat ToEngineIndexFormat(NativeIndexFormat nativeFormat) noexcept
		{
#if defined(Z_D3D12)
			return (nativeFormat == DXGI_FORMAT_R16_UINT) ? eIndexFormat::UInt16 : eIndexFormat::UInt32;
#elif defined(Z_VULKAN)
			return (nativeFormat == VK_INDEX_TYPE_UINT16) ? eIndexFormat::UInt16 : eIndexFormat::UInt32;
#elif defined(Z_METAL)
			return (nativeFormat == MTLIndexTypeUInt16) ? eIndexFormat::UInt16 : eIndexFormat::UInt32;
#else
			return static_cast<eIndexFormat>(nativeFormat);
#endif
		}

		// --- Шаблонная обертка ToEngine<T> ---

		template<typename T>
		[[nodiscard]] static constexpr T ToEngine(auto nativeFormat) noexcept
		{
			if constexpr (std::is_same_v<T, ePixelFormat>)
			{
				return ToEnginePixelFormat(static_cast<NativePixelFormat>(nativeFormat));
			}
			else if constexpr (std::is_same_v<T, eIndexFormat>)
			{
				return ToEngineIndexFormat(static_cast<NativeIndexFormat>(nativeFormat));
			}
		}
	};
}
