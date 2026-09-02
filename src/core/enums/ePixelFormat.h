#pragma once

#include <string_view>
#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	/**
	 * @enum ePixelFormat
	 * @brief Унифицированный кроссплатформенный формат пикселей и текстурных буферов (2 байта).
	 *
	 * @details Поддерживается нативно и аппаратно на всех целевых GAPI (DirectX 12, Vulkan, Metal)
	 *          и платформах (Windows, Linux, macOS, Android, iOS).
	 */
	enum class ePixelFormat : zU16
	{
		Unknown = 0,

		// --- 1. Универсальные 8-битные цветовые форматы (100% везде) ---
		R8_UNORM,           ///< 1 байт на пиксель: маски, шрифты, альфа
		RGBA8_UNORM,        ///< 4 байта на пиксель: стандартные текстуры (DX12 / Vulkan / Metal)
		RGBA8_SRGB,         ///< 4 байта на пиксель: текстуры с гамма-коррекцией sRGB
		BGRA8_UNORM,        ///< 4 байта на пиксель: нативный формат Swapchain (Windows, Android, Apple)
		BGRA8_SRGB,         ///< 4 байта на пиксель: sRGB Swapchain

		// --- 2. HDR и данные с плавающей запятой (100% везде) ---
		RGBA16_FLOAT,       ///< 8 байт на пиксель: HDR буферы кадра, постобработка
		R32_FLOAT,          ///< 4 байта на пиксель: скалярные данные, карты высот

		// --- 3. Буферы глубины и трафарета (100% везде) ---
		D32_FLOAT,          ///< 32-бит глубина (универсальный стандарт)
		D24_UNORM_S8_UINT,  ///< 24-бит глубина + 8-бит трафарет
		D32_FLOAT_S8_UINT,  ///< 32-бит глубина + 8-бит трафарет (дефолтный буфер Vulkan/Metal)

		// --- 4. Сжатые форматы Desktop (Windows / Linux / macOS) ---
		BC1_UNORM,          ///< DXT1 (RGB без альфы / 1-бит альфа)
		BC3_UNORM,          ///< DXT5 (RGBA с интерполированной альфой)
		BC7_UNORM,          ///< Высококачественное сжатие RGBA

		// --- 5. Сжатые форматы Mobile (Android / iOS) ---
		ASTC_4x4_UNORM,     ///< Универсальный мобильный стандарт качества (Android + iOS)
		ETC2_RGBA8_UNORM    ///< Базовый мобильный стандарт OpenGL ES 3.0 / Vulkan
	};

	namespace PixelFormatUtils
	{
		/**
		 * @brief Возвращает размер одного не сжатого пикселя в байтах (для расчета размера буфера загрузки).
		 * @note Для сжатых форматов (BC1..BC7, ASTC, ETC2) возвращает 0 (размер блока вычисляется отдельно).
		 */
		[[nodiscard]] constexpr zU32 GetPixelFormatBytesPerPixel(ePixelFormat format) noexcept
		{
			switch (format)
			{
			case ePixelFormat::R8_UNORM:
				return 1;

			case ePixelFormat::RGBA8_UNORM:
			case ePixelFormat::RGBA8_SRGB:
			case ePixelFormat::BGRA8_UNORM:
			case ePixelFormat::BGRA8_SRGB:
			case ePixelFormat::R32_FLOAT:
			case ePixelFormat::D32_FLOAT:
			case ePixelFormat::D24_UNORM_S8_UINT:
				return 4;

			case ePixelFormat::RGBA16_FLOAT:
			case ePixelFormat::D32_FLOAT_S8_UINT:
				return 8;

			default:
				return 0;
			}
		}

		/// @brief Проверяет, является ли формат буфером глубины.
		[[nodiscard]] constexpr bool IsDepthFormat(ePixelFormat format) noexcept
		{
			return format == ePixelFormat::D32_FLOAT || 
			       format == ePixelFormat::D24_UNORM_S8_UINT || 
			       format == ePixelFormat::D32_FLOAT_S8_UINT;
		}

		/// @brief Проверяет, является ли формат буфером с трафаретом (Stencil).
		[[nodiscard]] constexpr bool IsStencilFormat(ePixelFormat format) noexcept
		{
			return format == ePixelFormat::D24_UNORM_S8_UINT || 
			       format == ePixelFormat::D32_FLOAT_S8_UINT;
		}

		/// @brief Проверяет, является ли формат sRGB (с аппаратной гамма-коррекцией).
		[[nodiscard]] constexpr bool IsSRGBFormat(ePixelFormat format) noexcept
		{
			return format == ePixelFormat::RGBA8_SRGB || format == ePixelFormat::BGRA8_SRGB;
		}

		/// @brief Проверяет, является ли формат сжатым (Block Compression / ASTC / ETC2).
		[[nodiscard]] constexpr bool IsCompressedFormat(ePixelFormat format) noexcept
		{
			switch (format)
			{
			case ePixelFormat::BC1_UNORM:
			case ePixelFormat::BC3_UNORM:
			case ePixelFormat::BC7_UNORM:
			case ePixelFormat::ASTC_4x4_UNORM:
			case ePixelFormat::ETC2_RGBA8_UNORM:
				return true;
			default:
				return false;
			}
		}
	}

	constexpr std::string_view ToString(ePixelFormat format)
	{
		switch (format)
		{
		case ePixelFormat::Unknown:           return "Unknown";
		case ePixelFormat::R8_UNORM:          return "R8_UNORM";
		case ePixelFormat::RGBA8_UNORM:       return "RGBA8_UNORM";
		case ePixelFormat::RGBA8_SRGB:        return "RGBA8_SRGB";
		case ePixelFormat::BGRA8_UNORM:       return "BGRA8_UNORM";
		case ePixelFormat::BGRA8_SRGB:        return "BGRA8_SRGB";
		case ePixelFormat::RGBA16_FLOAT:      return "RGBA16_FLOAT";
		case ePixelFormat::R32_FLOAT:         return "R32_FLOAT";
		case ePixelFormat::D32_FLOAT:         return "D32_FLOAT";
		case ePixelFormat::D24_UNORM_S8_UINT: return "D24_UNORM_S8_UINT";
		case ePixelFormat::D32_FLOAT_S8_UINT: return "D32_FLOAT_S8_UINT";
		case ePixelFormat::BC1_UNORM:         return "BC1_UNORM";
		case ePixelFormat::BC3_UNORM:         return "BC3_UNORM";
		case ePixelFormat::BC7_UNORM:         return "BC7_UNORM";
		case ePixelFormat::ASTC_4x4_UNORM:    return "ASTC_4x4_UNORM";
		case ePixelFormat::ETC2_RGBA8_UNORM:  return "ETC2_RGBA8_UNORM";
		}
		THROW_RUNTIME("Необработанный ePixelFormat");
	}
}
