#include "StandardExternalPakBuilder.h"
#include <fstream>
#include <format>
#include "core/constants/PackagesConstants.h"
#include "core/serialize/Serializer.h"
#include "../../AssetImporterRegistry.h"
#include "../../IAssetImporter.h"

namespace zzz::builder
{
	PakBuildResult StandardExternalPakBuilder::Build(
		const StagePackPlan& plan,
		const std::filesystem::path& outputDir)
	{
		PakBuildResult combinedResult{ .success = true };

		for (const auto& [type, assets] : plan.assetsByPak)
		{
			if (type != core::eDataDatType::Mesh &&
				type != core::eDataDatType::Material &&
				type != core::eDataDatType::Shader &&
				type != core::eDataDatType::Animation &&
				!assets.empty())
			{
				auto extRes = BuildPak(type, assets, plan, outputDir);
				if (!extRes.success)
				{
					return extRes;
				}

				combinedResult.entries.insert(combinedResult.entries.end(), extRes.entries.begin(), extRes.entries.end());
			}
		}

		return combinedResult;
	}

	PakBuildResult StandardExternalPakBuilder::BuildPak(
		core::eDataDatType type,
		std::span<const ProjectAssetInfo> assets,
		const StagePackPlan& plan,
		const std::filesystem::path& outputDir)
	{
		PakBuildResult result;

		const auto pakFileName = core::GetPakFileName(type);
		if (pakFileName.empty())
		{
			result.success = false;
			result.errorMessage = std::format("Тип '{}' не является типом внешнего пакета N.dat", static_cast<uint32_t>(type));
			return result;
		}

		result.outputFilePath = outputDir / pakFileName;

		std::vector<std::byte> pakBytes;
		pakBytes.reserve(4096 + assets.size() * 1024);

		// 1. Заголовок PakFileHeader (4096 байт)
		core::PakFileHeader header(core::c_PakFileFormat, static_cast<zU32>(assets.size()), plan.buildTimestamp);
		core::Serializer serializer;
		auto serHeaderRes = serializer.Serialize(pakBytes, header);
		if (!serHeaderRes)
		{
			result.success = false;
			result.errorMessage = std::format("Ошибка сериализации PakFileHeader: {}", serHeaderRes.error());
			return result;
		}

		if (pakBytes.size() != core::PakFileHeader::c_FullHeaderSize)
		{
			result.success = false;
			result.errorMessage = std::format("Неверный размер заголовка PakFileHeader: {} (ожидалось 4096 байт)", pakBytes.size());
			return result;
		}

		// 2. Блобы данных с выравниванием по 16 байтам
		for (const auto& asset : assets)
		{
			// Выравнивание смещения по 16 байтам
			const std::size_t unalignedOffset = pakBytes.size();
			const std::size_t padding = (16 - (unalignedOffset % 16)) % 16;
			if (padding > 0)
			{
				pakBytes.insert(pakBytes.end(), padding, std::byte{ 0 });
			}

			const zU64 blobOffset = static_cast<zU64>(pakBytes.size());

			// Чтение исходного файла
			std::ifstream srcFile(asset.fullPath, std::ios::binary);
			if (!srcFile.is_open())
			{
				result.success = false;
				result.errorMessage = std::format("Не удалось открыть файл ресурса для чтения: {}", asset.fullPath.string());
				return result;
			}

			srcFile.seekg(0, std::ios::end);
			const auto fileSize = srcFile.tellg();
			srcFile.seekg(0, std::ios::beg);

			std::vector<std::byte> srcBytes(static_cast<size_t>(fileSize));
			if (fileSize > 0)
			{
				srcFile.read(reinterpret_cast<char*>(srcBytes.data()), fileSize);
			}

			std::vector<std::byte> payload;
			core::AssetMetadata metadata{};

			const auto ext = asset.fullPath.extension().string();
			auto importer = AssetImporterRegistry::Instance().GetImporter(ext);
			if (importer)
			{
				ImportContext ctx{
					.sourceFilePath = asset.fullPath,
					.metaFilePath = asset.metaPath,
					.assetGuid = asset.guid,
					.assetName = asset.relativePath.stem().string(),
					.targetPlatform = plan.targetPlatform,
					.sourceData = srcBytes
				};

				auto importRes = importer->Import(ctx);
				if (!importRes)
				{
					result.success = false;
					result.errorMessage = std::format("Ошибка импорта ресурса {}: {}", asset.relativePath.string(), importRes.error());
					return result;
				}

				payload = std::move(importRes->binaryPayload);
				metadata = importRes->metadata;
			}
			else
			{
				result.success = false;
				result.errorMessage = std::format("Ресурс '{}' [{}] не имеет зарегистрированного импортёра",
					asset.relativePath.string(), asset.guid.ToString());
				return result;
			}

			const zU64 blobSize = static_cast<zU64>(payload.size());

			// Формируем запись TOC
			result.entries.emplace_back(asset.guid, static_cast<zU32>(type), blobOffset, blobSize, metadata);

			// Записываем полезную нагрузку
			pakBytes.insert(pakBytes.end(), payload.begin(), payload.end());
		}

		// 3. Запись на диск
		std::ofstream outFile(result.outputFilePath, std::ios::binary | std::ios::trunc);
		if (!outFile.is_open())
		{
			result.success = false;
			result.errorMessage = std::format("Не удалось создать выходной файл пака: {}", result.outputFilePath.string());
			return result;
		}

		if (!pakBytes.empty())
		{
			outFile.write(reinterpret_cast<const char*>(pakBytes.data()), pakBytes.size());
		}
		outFile.flush();

		result.success = true;
		result.outputFileSize = pakBytes.size();
		return result;
	}
}
