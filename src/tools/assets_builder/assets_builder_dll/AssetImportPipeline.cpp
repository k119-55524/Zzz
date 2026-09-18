#include "AssetImportPipeline.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <json.hpp>

#include "AssetImporterRegistry.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace zzz::builder
{
	namespace
	{
		std::expected<std::vector<std::byte>, std::string> ReadFileFully(const fs::path& path)
		{
			std::ifstream file(path, std::ios::binary | std::ios::ate);
			if (!file.is_open())
				return std::unexpected("Не удалось открыть файл '" + path.string() + "'.");

			const std::streampos end = file.tellg();
			if (end < 0)
				return std::unexpected("Не удалось определить размер файла '" + path.string() + "'.");

			std::vector<std::byte> bytes(static_cast<std::size_t>(end));
			file.seekg(0, std::ios::beg);
			if (!bytes.empty() && !file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())))
				return std::unexpected("Не удалось полностью прочитать файл '" + path.string() + "'.");

			return bytes;
		}

		std::expected<core::Guid, std::string> ReadMetaGuid(const fs::path& metaPath)
		{
			auto bytesRes = ReadFileFully(metaPath);
			if (!bytesRes)
				return std::unexpected(bytesRes.error());

			const auto& bytes = *bytesRes;
			const std::string source(reinterpret_cast<const char*>(bytes.data()), bytes.size());
			json root = json::parse(source, nullptr, false);
			if (root.is_discarded())
				return std::unexpected("Мета-файл содержит некорректный JSON: '" + metaPath.string() + "'.");
			if (!root.is_object())
				return std::unexpected("Корень мета-файла должен быть JSON-объектом: '" + metaPath.string() + "'.");
			if (!root.contains("guid") || !root["guid"].is_string())
				return std::unexpected("Мета-файл не содержит строковое поле 'guid': '" + metaPath.string() + "'.");

			auto guid = core::Guid::Parse(root["guid"].get<std::string>());
			if (!guid || !guid->IsValid())
				return std::unexpected("Мета-файл содержит невалидный GUID: '" + metaPath.string() + "'.");

			return *guid;
		}
	}

	std::expected<std::optional<ImportedDataAsset>, std::string> AssetImportPipeline::TryImport(
		const fs::path& sourceFilePath,
		core::eTargetPlatform targetPlatform)
	{
		auto importer = AssetImporterRegistry::Instance().GetImporter(sourceFilePath.extension().string());
		if (!importer)
			return std::optional<ImportedDataAsset>{};

		fs::path metaPath = sourceFilePath;
		metaPath += ".meta";
		if (!fs::exists(metaPath) || !fs::is_regular_file(metaPath))
			return std::unexpected("Для зарегистрированного ресурса отсутствует мета-файл '" + metaPath.string() + "'.");

		auto guidRes = ReadMetaGuid(metaPath);
		if (!guidRes)
			return std::unexpected(guidRes.error());

		auto sourceRes = ReadFileFully(sourceFilePath);
		if (!sourceRes)
			return std::unexpected(sourceRes.error());

		ImportContext context{
			.sourceFilePath = sourceFilePath,
			.metaFilePath = metaPath,
			.assetGuid = *guidRes,
			.assetName = sourceFilePath.stem().string(),
			.targetPlatform = targetPlatform,
			.sourceData = *sourceRes
		};

		try
		{
			auto importRes = importer->Import(context);
			if (!importRes)
				return std::unexpected(importRes.error());
			if (importRes->binaryPayload.empty())
				return std::unexpected("Импортёр вернул пустой payload для ресурса '" + sourceFilePath.string() + "'.");

			return std::optional<ImportedDataAsset>{ ImportedDataAsset{
				.name = std::move(context.assetName),
				.guid = context.assetGuid,
				.resourceType = importer->GetResourceType(),
				.payload = std::move(importRes->binaryPayload)
			} };
		}
		catch (const std::exception& ex)
		{
			return std::unexpected("Исключение импортёра для '" + sourceFilePath.string() + "': " + ex.what());
		}
		catch (...)
		{
			return std::unexpected("Неизвестное исключение импортёра для '" + sourceFilePath.string() + "'.");
		}
	}
}
