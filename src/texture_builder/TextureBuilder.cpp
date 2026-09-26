
#include "TextureBuilder.h"

Z_SET_LOG_CATEGORY(::zzz::core::Assets);

namespace zzz::texture
{
#if Z_DESKTOP
	namespace
	{
		DXGI_FORMAT ToDxgiFormat(core::ePixelFormat format)
		{
			switch (format) {
			case core::ePixelFormat::RGBA8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
			case core::ePixelFormat::RGBA8_SRGB:  return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			case core::ePixelFormat::BC1_UNORM:   return DXGI_FORMAT_BC1_UNORM;
			case core::ePixelFormat::BC1_SRGB:    return DXGI_FORMAT_BC1_UNORM_SRGB;
			case core::ePixelFormat::BC3_UNORM:   return DXGI_FORMAT_BC3_UNORM;
			case core::ePixelFormat::BC3_SRGB:    return DXGI_FORMAT_BC3_UNORM_SRGB;
			case core::ePixelFormat::BC4_UNORM:   return DXGI_FORMAT_BC4_UNORM;
			case core::ePixelFormat::BC4_SNORM:   return DXGI_FORMAT_BC4_SNORM;
			case core::ePixelFormat::BC5_UNORM:   return DXGI_FORMAT_BC5_UNORM;
			case core::ePixelFormat::BC5_SNORM:   return DXGI_FORMAT_BC5_SNORM;
			case core::ePixelFormat::BC7_UNORM:   return DXGI_FORMAT_BC7_UNORM;
			case core::ePixelFormat::BC7_SRGB:    return DXGI_FORMAT_BC7_UNORM_SRGB;
			default:                              return DXGI_FORMAT_UNKNOWN;
			}
		}
	}
#endif

	TextureBuilder::TextureBuilder(const TextureBuilderConfig& config)
		: m_config(config)
	{
	}

	std::expected<ImageInfo, std::string> TextureBuilder::Probe(std::span<const zU8> fileBytes) const
	{
		if (fileBytes.empty()) {
			return std::unexpected("Input file byte span is empty");
		}

		int w = 0;
		int h = 0;
		int comp = 0;
		const int result = stbi_info_from_memory(
			fileBytes.data(),
			static_cast<int>(fileBytes.size()),
			&w, &h, &comp
		);

		if (result == 0) {
			DOutWarning("TextureBuilder::Probe: stbi_info failed to parse image header");
			return std::unexpected("stbi_info failed to parse image header (unsupported or corrupted format)");
		}

		ImageInfo info;
		info.width = static_cast<zU32>(w);
		info.height = static_cast<zU32>(h);
		info.channels = static_cast<zU32>(comp);
		info.bitDepth = 8;
		info.hasAlpha = (comp == 2 || comp == 4);

		DOut("TextureBuilder::Probe: parsed image {}x{}, channels={}, hasAlpha={}, divisibleByTwo={}",
			info.width, info.height, info.channels, info.hasAlpha, info.IsDivisibleByTwo());

		return info;
	}

	TextureConvertResult TextureBuilder::Convert(
		std::span<const zU8> fileBytes,
		const TextureConvertOptions& options
	) const
	{
		if (fileBytes.empty()) {
			DOutError("TextureBuilder::Convert: input file bytes buffer is empty");
			return { .success = false, .errorMessage = "Input file bytes buffer is empty" };
		}

		DOut("TextureBuilder::Convert: starting conversion, targetFormat={}, mips={}",
			static_cast<zU32>(options.targetFormat), options.generateMips);

		// Декодирование пикселей (принудительно в 4 канала RGBA8)
		int w = 0;
		int h = 0;
		int comp = 0;
		unsigned char* rawPixels = stbi_load_from_memory(
			fileBytes.data(),
			static_cast<int>(fileBytes.size()),
			&w, &h, &comp, 4
		);

		if (!rawPixels) {
			const std::string reason = stbi_failure_reason() ? stbi_failure_reason() : "unknown";
			DOutError("TextureBuilder::Convert: failed to decode image: {}", reason);
			return {
				.success = false,
				.errorMessage = std::format("Failed to decode image data: {}", reason)
			};
		}

		if (w <= 0 || h <= 0) {
			stbi_image_free(rawPixels);
			DOutError("TextureBuilder::Convert: invalid dimensions ({}x{})", w, h);
			return { .success = false, .errorMessage = "Image has invalid dimensions (width or height <= 0)" };
		}

		// Валидация максимального допустимого размера из конфигурации
		if (static_cast<zU32>(w) > m_config.maxDimension || static_cast<zU32>(h) > m_config.maxDimension) {
			const std::string msg = std::format("Image dimensions ({}x{}) exceed maximum allowed dimension ({})",
				w, h, m_config.maxDimension);
			stbi_image_free(rawPixels);
			DOutError("TextureBuilder::Convert: validation error: {}", msg);
			return { .success = false, .errorMessage = msg };
		}

		// 2. Валидация кратности 2 (если включено в конфигурации)
		if (m_config.requireDivisibleByTwo && ((w % 2 != 0) || (h % 2 != 0))) {
			const std::string msg = std::format("Image dimensions ({}x{}) must be divisible by 2", w, h);
			stbi_image_free(rawPixels);
			DOutError("TextureBuilder::Convert: validation error: {}", msg);
			return { .success = false, .errorMessage = msg };
		}

		const bool compressed = core::PixelFormatUtils::IsCompressedFormat(options.targetFormat);

#if !Z_DESKTOP
		if (compressed) {
			stbi_image_free(rawPixels);
			DOutError("TextureBuilder::Convert: BCn compression is not supported on this platform. ASTC compression is required for mobile platforms.");
			return {
				.success = false,
				.errorMessage = "BCn compression is only supported on Desktop platforms (Windows/Linux/macOS) via DirectXTex. Mobile platforms (Android/iOS) require ASTC compression."
			};
		}
#endif

		// Формирование цепочки мипмапов в сыром RGBA8
		struct RawMip
		{
			zU32 width{ 0 };
			zU32 height{ 0 };
			std::vector<zU8> rgbaPixels;
		};

		std::vector<RawMip> rawMips;
		zU32 currentW = static_cast<zU32>(w);
		zU32 currentH = static_cast<zU32>(h);

		rawMips.push_back(RawMip{
			.width = currentW,
			.height = currentH,
			.rgbaPixels = std::vector<zU8>(rawPixels, rawPixels + (currentW * currentH * 4))
			});
		stbi_image_free(rawPixels);

		const bool isSRGB = core::PixelFormatUtils::IsSRGBFormat(options.targetFormat);

		if (options.generateMips) {
			while (currentW > 1 || currentH > 1) {
				const zU32 nextW = std::max(1u, currentW / 2);
				const zU32 nextH = std::max(1u, currentH / 2);
				std::vector<zU8> nextPixels(nextW * nextH * 4);

				if (isSRGB) {
					stbir_resize_uint8_srgb(
						rawMips.back().rgbaPixels.data(), static_cast<int>(currentW), static_cast<int>(currentH), static_cast<int>(currentW * 4),
						nextPixels.data(), static_cast<int>(nextW), static_cast<int>(nextH), static_cast<int>(nextW * 4),
						STBIR_RGBA
					);
				}
				else {
					stbir_resize_uint8_linear(
						rawMips.back().rgbaPixels.data(), static_cast<int>(currentW), static_cast<int>(currentH), static_cast<int>(currentW * 4),
						nextPixels.data(), static_cast<int>(nextW), static_cast<int>(nextH), static_cast<int>(nextW * 4),
						STBIR_RGBA
					);
				}

				rawMips.push_back(RawMip{
					.width = nextW,
					.height = nextH,
					.rgbaPixels = std::move(nextPixels)
					});

				currentW = nextW;
				currentH = nextH;
			}
		}

		DOut("TextureBuilder::Convert: generated {} raw mip levels", rawMips.size());

		// Сжатие или копирование каждого мип-уровня
		TextureConvertResult result;
		result.width = static_cast<zU32>(w);
		result.height = static_cast<zU32>(h);
		result.format = options.targetFormat;

#if Z_DESKTOP
		const DXGI_FORMAT targetDxgi = compressed ? ToDxgiFormat(options.targetFormat) : DXGI_FORMAT_UNKNOWN;
#endif

		for (const auto& mip : rawMips) {
			TextureMipDesc desc;
			desc.width = mip.width;
			desc.height = mip.height;
			desc.byteOffset = result.payload.size();

			if (!compressed) {
				// Несжатый RGBA8
				desc.rowPitch = mip.width * 4;
				desc.byteSize = mip.rgbaPixels.size();
				result.payload.insert(result.payload.end(), mip.rgbaPixels.begin(), mip.rgbaPixels.end());
			}
			else {
#if Z_DESKTOP
				// Компрессия через DirectXTex
				DirectX::Image srcImage;
				srcImage.width = mip.width;
				srcImage.height = mip.height;
				srcImage.format = isSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
				srcImage.rowPitch = mip.width * 4;
				srcImage.slicePitch = mip.rgbaPixels.size();
				srcImage.pixels = const_cast<uint8_t*>(mip.rgbaPixels.data());

				DirectX::ScratchImage compressedImage;
				const HRESULT hr = DirectX::Compress(
					srcImage,
					targetDxgi,
					DirectX::TEX_COMPRESS_DEFAULT,
					DirectX::TEX_THRESHOLD_DEFAULT,
					compressedImage
				);

				if (FAILED(hr)) {
					DOutError("TextureBuilder::Convert: DirectXTex compression failed for mip {}x{}, HRESULT: 0x{:08X}",
						mip.width, mip.height, static_cast<zU32>(hr));
					return {
						.success = false,
						.errorMessage = std::format("DirectXTex compression failed for mip {}x{}, HRESULT: 0x{:08X}",
													mip.width, mip.height, static_cast<zU32>(hr))
					};
				}

				const auto* image = compressedImage.GetImage(0, 0, 0);
				desc.rowPitch = static_cast<zU32>(image->rowPitch);
				desc.byteSize = compressedImage.GetPixelsSize();

				const auto* compressedBytes = compressedImage.GetPixels();
				result.payload.insert(result.payload.end(), compressedBytes, compressedBytes + desc.byteSize);
#endif
			}

			result.mips.push_back(desc);
		}

		result.success = true;
		DOut("TextureBuilder::Convert: finished successfully ({}x{}, {} mips, total payload {} bytes)",
			result.width, result.height, result.mips.size(), result.payload.size());

		return result;
	}

	AtlasBuildResult TextureBuilder::BuildAtlas(const std::vector<std::filesystem::path>& /*inputFiles*/, const std::filesystem::path& /*outputImagePath*/, const AtlasOptions& /*options*/) const
	{
		return
		{
			.success = false,
			.errorMessage = "TextureBuilder::BuildAtlas is not implemented yet"
		};
	}

	TextureConvertResult TextureBuilder::PackChannels(const ChannelPackSources& /*sources*/, const TextureConvertOptions& /*options*/) const
	{
		return
		{
			.success = false,
			.errorMessage = "TextureBuilder::PackChannels is not implemented yet"
		};
	}

	TextureConvertResult TextureBuilder::ProcessNormalMap(const std::filesystem::path& /*filePath*/, const NormalMapOptions& /*options*/) const
	{
		return
		{
			.success = false,
			.errorMessage = "TextureBuilder::ProcessNormalMap is not implemented yet"
		};
	}

	TextureConvertResult TextureBuilder::BuildCubemap(const std::array<std::filesystem::path, 6>& /*faceFiles*/, const TextureConvertOptions& /*options*/) const
	{
		return
		{
			.success = false,
			.errorMessage = "TextureBuilder::BuildCubemap is not implemented yet"
		};
	}
} 
