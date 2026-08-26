#pragma once

#if defined(Z_METAL)
#import <Metal/Metal.h>
#include <format>
#include <string_view>

namespace zzz::engine::gapi
{
	inline std::string_view ToString(MTLPixelFormat format) noexcept
	{
		switch (format)
		{
		case MTLPixelFormatInvalid:                 return "MTLPixelFormat::MTLPixelFormatInvalid";
		case MTLPixelFormatRGBA8Unorm:              return "MTLPixelFormat::MTLPixelFormatRGBA8Unorm";
		case MTLPixelFormatRGBA8Unorm_sRGB:         return "MTLPixelFormat::MTLPixelFormatRGBA8Unorm_sRGB";
		case MTLPixelFormatBGRA8Unorm:              return "MTLPixelFormat::MTLPixelFormatBGRA8Unorm";
		case MTLPixelFormatBGRA8Unorm_sRGB:         return "MTLPixelFormat::MTLPixelFormatBGRA8Unorm_sRGB";
		case MTLPixelFormatDepth32Float:            return "MTLPixelFormat::MTLPixelFormatDepth32Float";
		case MTLPixelFormatDepth24Unorm_Stencil8:   return "MTLPixelFormat::MTLPixelFormatDepth24Unorm_Stencil8";
		case MTLPixelFormatDepth32Float_Stencil8:   return "MTLPixelFormat::MTLPixelFormatDepth32Float_Stencil8";
		default: break;
		}
		return "MTLPixelFormat::UNKNOWN_VALUE";
	}
}

template <>
struct std::formatter<MTLPixelFormat> : std::formatter<std::string_view>
{
	auto format(MTLPixelFormat format, std::format_context& ctx) const
	{
		return std::formatter<std::string_view>::format(zzz::engine::gapi::ToString(format), ctx);
	}
};

#endif // Z_METAL
