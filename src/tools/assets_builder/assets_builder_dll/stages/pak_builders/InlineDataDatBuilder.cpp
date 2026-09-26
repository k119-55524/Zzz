#include "InlineDataDatBuilder.h"
#include <fstream>
#include <format>
#include <set>
#include <algorithm>
#include "core/constants/PackagesConstants.h"
#include "core/serialize/Serializer.h"
#include "../../AssetImporterRegistry.h"
#include "../../IAssetImporter.h"

namespace zzz::builder
{
	PakBuildResult InlineDataDatBuilder::BuildDataDat(
		const StagePackPlan& plan,
		std::span<const core::PackageEntry> externalEntries,
		const std::filesystem::path& outputDir)
	{
		PakBuildResult result;
		result.outputFilePath = outputDir / "data.dat";

		// 1. Формируем Таблицу 1: уникальные отсортированные типы внешних паков
		std::set<core::eDataDatType> uniquePackTypes;
		for (const auto& extEntry : externalEntries)
		{
			uniquePackTypes.insert(static_cast<core::eDataDatType>(extEntry.GetAssetType()));
		}
		std::vector<core::eDataDatType> externalPackTypes(uniquePackTypes.begin(), uniquePackTypes.end());
		const zU32 packCount = static_cast<zU32>(externalPackTypes.size());

		// 2. Собираем список встроенных ассетов (Mesh, Material, Shader, Animation)
		std::vector<std::pair<core::eDataDatType, const ProjectAssetInfo*>> inlineAssets;
		for (const auto& [type, assets] : plan.assetsByPak)
		{
			if (type == core::eDataDatType::Mesh ||
				type == core::eDataDatType::Material ||
				type == core::eDataDatType::Shader ||
				type == core::eDataDatType::Animation)
			{
				for (const auto& asset : assets)
				{
					inlineAssets.emplace_back(type, &asset);
				}
			}
		}

		const zU32 inlineCount = static_cast<zU32>(inlineAssets.size());
		const zU32 externalCount = static_cast<zU32>(externalEntries.size());
		const zU32 totalEntryCount = inlineCount + externalCount;

		// 3. Вычисляем размеры оглавления и смещение payload
		const std::size_t headerSize = core::DataDatHeader::c_FullHeaderSize; // 64
		const std::size_t table1Size = packCount * sizeof(zU32);              // 4 байта на элемент
		const std::size_t table2Size = inlineCount * core::PackageEntry::BinarySize(); // 68 байт на элемент
		const std::size_t table3Size = externalCount * core::PackageEntry::BinarySize(); // 68 байт на элемент

		const std::size_t tocEnd = headerSize + table1Size + table2Size + table3Size;
		const std::size_t payloadBegin = (tocEnd + 15) & ~static_cast<std::size_t>(15);
		const std::size_t tocPadding = payloadBegin - tocEnd;

		// 4. Запекаем встроенные блобы и формируем записи Таблицы 2
		std::vector<core::PackageEntry> inlineEntries;
		inlineEntries.reserve(inlineCount);

		std::vector<std::byte> payloadBytes;
		std::size_t currentBlobOffset = payloadBegin;

		for (const auto& [type, pAsset] : inlineAssets)
		{
			const auto& asset = *pAsset;

			// Читаем исходный файл
			std::ifstream srcFile(asset.fullPath, std::ios::binary);
			if (!srcFile.is_open())
			{
				result.success = false;
				result.errorMessage = std::format("Не удалось открыть файл inline ассета: {}", asset.fullPath.string());
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

			std::vector<std::byte> blobData;
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
					result.errorMessage = std::format("Ошибка импорта inline ассета {}: {}", asset.relativePath.string(), importRes.error());
					return result;
				}

				blobData = std::move(importRes->binaryPayload);
				metadata = importRes->metadata;
			}
			else
			{
				result.success = false;
				result.errorMessage = std::format("Inline ассет '{}' [{}] не имеет зарегистрированного импортёра",
					asset.relativePath.string(), asset.guid.ToString());
				return result;
			}

			const zU64 blobSize = static_cast<zU64>(blobData.size());
			inlineEntries.emplace_back(asset.guid, static_cast<zU32>(type), static_cast<zU64>(currentBlobOffset), blobSize, metadata);

			// Добавляем блоб в payload
			payloadBytes.insert(payloadBytes.end(), blobData.begin(), blobData.end());
			currentBlobOffset += blobSize;

			// Выравнивание следующего блоба по 16 байтам
			const std::size_t alignPadding = (16 - (currentBlobOffset % 16)) % 16;
			if (alignPadding > 0)
			{
				payloadBytes.insert(payloadBytes.end(), alignPadding, std::byte{ 0 });
				currentBlobOffset += alignPadding;
			}
		}

		// 5. Сериализуем архив целиком
		std::vector<std::byte> dataDatBytes;
		dataDatBytes.reserve(payloadBegin + payloadBytes.size());

		// 5.1. Заголовок DataDatHeader (64 байта)
		core::DataDatHeaderExtra extra{
			.packCount = packCount,
			.inlineCount = inlineCount,
			.externalCount = externalCount
		};
		core::DataDatHeader header(core::c_DataDatFormat, totalEntryCount, plan.buildTimestamp, extra);

		core::Serializer serializer;
		auto serHeaderRes = serializer.Serialize(dataDatBytes, header);
		if (!serHeaderRes)
		{
			result.success = false;
			result.errorMessage = std::format("Ошибка сериализации DataDatHeader: {}", serHeaderRes.error());
			return result;
		}

		if (dataDatBytes.size() != core::DataDatHeader::c_FullHeaderSize)
		{
			result.success = false;
			result.errorMessage = std::format("Неверный размер заголовка DataDatHeader: {} (ожидалось 64 байта)", dataDatBytes.size());
			return result;
		}

		// 5.2. Таблица 1: типы внешних паков (packCount элементов по 4 байта)
		for (const auto pakType : externalPackTypes)
		{
			const zU32 typeVal = static_cast<zU32>(pakType);
			if (auto serRes = serializer.Serialize(dataDatBytes, typeVal); !serRes)
			{
				result.success = false;
				result.errorMessage = std::format("Ошибка сериализации элемента Таблицы 1 data.dat: {}", serRes.error());
				return result;
			}
		}

		// 5.3. Таблица 2: встроенные записи PackageEntry
		for (const auto& entry : inlineEntries)
		{
			if (auto serRes = serializer.Serialize(dataDatBytes, entry); !serRes)
			{
				result.success = false;
				result.errorMessage = std::format("Ошибка сериализации элемента Таблицы 2 data.dat: {}", serRes.error());
				return result;
			}
		}

		// 5.4. Таблица 3: внешние записи PackageEntry
		for (const auto& entry : externalEntries)
		{
			if (auto serRes = serializer.Serialize(dataDatBytes, entry); !serRes)
			{
				result.success = false;
				result.errorMessage = std::format("Ошибка сериализации элемента Таблицы 3 data.dat: {}", serRes.error());
				return result;
			}
		}

		// 5.5. Паддинг между TOC и полезной нагрузкой до 16 байт
		if (tocPadding > 0)
		{
			dataDatBytes.insert(dataDatBytes.end(), tocPadding, std::byte{ 0 });
		}

		if (dataDatBytes.size() != payloadBegin)
		{
			result.success = false;
			result.errorMessage = std::format("Рассинхронизация смещения начала полезной нагрузки: {} != {}", dataDatBytes.size(), payloadBegin);
			return result;
		}

		// 5.6. Полезная нагрузка
		dataDatBytes.insert(dataDatBytes.end(), payloadBytes.begin(), payloadBytes.end());

		// 6. Запись на диск
		std::ofstream outFile(result.outputFilePath, std::ios::binary | std::ios::trunc);
		if (!outFile.is_open())
		{
			result.success = false;
			result.errorMessage = std::format("Не удалось создать выходной файл data.dat: {}", result.outputFilePath.string());
			return result;
		}

		if (!dataDatBytes.empty())
		{
			outFile.write(reinterpret_cast<const char*>(dataDatBytes.data()), dataDatBytes.size());
		}
		outFile.flush();

		result.success = true;
		result.entries = std::move(inlineEntries);
		result.outputFileSize = dataDatBytes.size();
		return result;
	}
}
