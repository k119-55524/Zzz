#include "TextureImporter.h"
#include <fstream>
#include <algorithm>
#include <cctype>
#include "json.hpp"
#include "core/logger/logger.h"
#include "core/utils/macros/MiscMacros.h"
#include "core/constants/LogCategoryConstants.h"

namespace zzz::builder
{
	namespace
	{
		[[nodiscard]] bool IsDesktopPlatform(core::eTargetPlatform platform) noexcept
		{
			return platform == core::eTargetPlatform::Windows ||
			       platform == core::eTargetPlatform::Linux ||
			       platform == core::eTargetPlatform::MacOS;
		}

		[[nodiscard]] std::string ToUpper(std::string_view str)
		{
			std::string res(str);
			std::ranges::transform(res, res.begin(), [](unsigned char c) {
				return static_cast<char>(std::toupper(c));
			});
			return res;
		}
	}

	std::expected<zzz::texture::ImageInfo, std::string> TextureImporter::Probe(std::span<const zU8> fileBytes) const
	{
		return m_Builder.Probe(fileBytes);
	}

	ImportResult TextureImporter::Import(const ImportContext& ctx)
	{
		if (ctx.sourceData.empty())
		{
			return UNEXPECTED("Исходные данные текстуры пусты: {}", ctx.sourceFilePath.string());
		}

		std::string textureTypeStr = "Color";
		bool generateMipMaps = true;
		zU32 maxTextureSize = 2048;
		std::string requestedFormatStr;

		// Читаем параметры из .meta
		if (!ctx.metaFilePath.empty() && std::filesystem::exists(ctx.metaFilePath))
		{
			std::ifstream metaFile(ctx.metaFilePath);
			if (metaFile.is_open())
			{
				try
				{
					nlohmann::json metaJson;
					metaFile >> metaJson;

					if (metaJson.contains("textureSettings") && metaJson["textureSettings"].is_object())
					{
						const auto& ts = metaJson["textureSettings"];
						if (ts.contains("textureType") && ts["textureType"].is_string())
							textureTypeStr = ts["textureType"].get<std::string>();
						if (ts.contains("generateMipMaps") && ts["generateMipMaps"].is_boolean())
							generateMipMaps = ts["generateMipMaps"].get<bool>();
						if (ts.contains("maxTextureSize") && ts["maxTextureSize"].is_number_unsigned())
							maxTextureSize = ts["maxTextureSize"].get<zU32>();
					}

					if (metaJson.contains("platformSettings") && metaJson["platformSettings"].is_object())
					{
						const auto& ps = metaJson["platformSettings"];
						const std::string platName(core::ToString(ctx.targetPlatform));
						const std::string famName = IsDesktopPlatform(ctx.targetPlatform) ? "Desktop" : "Mobile";

						if (ps.contains(platName) && ps[platName].is_object() && ps[platName].contains("format"))
						{
							requestedFormatStr = ps[platName]["format"].get<std::string>();
						}
						else if (ps.contains(famName) && ps[famName].is_object() && ps[famName].contains("format"))
						{
							requestedFormatStr = ps[famName]["format"].get<std::string>();
						}
					}
				}
				catch (const std::exception& e)
				{
					DOut(Assets, "Внимание: ошибка чтения .meta для текстуры {}: {}", ctx.sourceFilePath.string(), e.what());
				}
			}
		}

		// Вывод гаммы и семантики из textureType
		const bool isColor = (textureTypeStr == "Color");
		const bool isNormalMap = (textureTypeStr == "NormalMap");
		const bool sRGB = isColor;

		const bool isDesktop = IsDesktopPlatform(ctx.targetPlatform);
		const std::string upperFmt = ToUpper(requestedFormatStr);

		core::ePixelFormat targetFormat = core::ePixelFormat::Unknown;

		if (isDesktop)
		{
			// Проверка на случай указания мобильного формата на десктопе
			if (upperFmt.starts_with("ASTC") || upperFmt.starts_with("ETC2"))
			{
				DOut(Assets, "Предупреждение: для Desktop-таргета указан мобильный формат '{}'. Заменён на авто-дефолт BCn.", requestedFormatStr);
			}
			else if (upperFmt == "BC7")
			{
				targetFormat = sRGB ? core::ePixelFormat::BC7_SRGB : core::ePixelFormat::BC7_UNORM;
			}
			else if (upperFmt == "BC5")
			{
				targetFormat = core::ePixelFormat::BC5_UNORM;
			}
			else if (upperFmt == "BC4")
			{
				targetFormat = core::ePixelFormat::BC4_UNORM;
			}
			else if (upperFmt == "BC1")
			{
				targetFormat = sRGB ? core::ePixelFormat::BC1_SRGB : core::ePixelFormat::BC1_UNORM;
			}
			else if (upperFmt == "BC3")
			{
				targetFormat = sRGB ? core::ePixelFormat::BC3_SRGB : core::ePixelFormat::BC3_UNORM;
			}
			else if (upperFmt == "RGBA8")
			{
				targetFormat = sRGB ? core::ePixelFormat::RGBA8_SRGB : core::ePixelFormat::RGBA8_UNORM;
			}

			// Если формат не распознан или не указан — применяем авто-дефолт
			if (targetFormat == core::ePixelFormat::Unknown)
			{
				if (isColor)
					targetFormat = core::ePixelFormat::BC7_SRGB;
				else if (isNormalMap)
					targetFormat = core::ePixelFormat::BC5_UNORM;
				else
					targetFormat = core::ePixelFormat::BC4_UNORM;
			}
		}
		else
		{
			// Мобильный таргет (Android/iOS): fallback в RGBA8
			DOut(Assets, "Предупреждение: для Mobile-таргета текстура '{}' запекается в RGBA8 (ASTC/ETC2 зарезервированы)",
				ctx.sourceFilePath.filename().string());
			targetFormat = sRGB ? core::ePixelFormat::RGBA8_SRGB : core::ePixelFormat::RGBA8_UNORM;
		}

		zzz::texture::TextureConvertOptions options{
			.targetFormat = targetFormat,
			.generateMips = generateMipMaps,
			.textureType = core::eTextureType::Texture2D,
			.isNormalMap = isNormalMap,
			.maxTextureSize = maxTextureSize
		};

		const std::span<const zU8> fileBytes(
			reinterpret_cast<const zU8*>(ctx.sourceData.data()),
			ctx.sourceData.size());

		auto convertRes = m_Builder.Convert(fileBytes, options);
		if (!convertRes)
		{
			return UNEXPECTED("Ошибка сжатия текстуры {}: {}", ctx.sourceFilePath.string(), convertRes.error());
		}

		ImportedAssetData result;
		result.binaryPayload.resize(convertRes->payload.size());
		std::memcpy(result.binaryPayload.data(), convertRes->payload.data(), convertRes->payload.size());
		result.metadata.texture = convertRes->metadata;

		return result;

	}
}
