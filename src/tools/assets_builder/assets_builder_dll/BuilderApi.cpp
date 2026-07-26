#include "BuilderApi.h"
#include <common/constants.h>
#include <common/package_format.h>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <cstring>

namespace fs = std::filesystem;

extern "C"
{
	BUILDER_API const char* GetBuilderEngineVersion()
	{
		return "1.0.0";
	}

	BUILDER_API const char* GetGamePackageFileName()
	{
		return zzz::common::c_GamePackageFileName.data();
	}

	BUILDER_API bool SerializeProjectManifest(const char* projectJsonPath, const char* outputBinaryPath)
	{
		if (!projectJsonPath || !outputBinaryPath)
			return false;

		fs::path projPath(projectJsonPath);
		fs::path outPath(outputBinaryPath);

		if (!fs::exists(projPath))
			return false;

		fs::path projectRootDir = projPath.parent_path();
		fs::path assetsDir = projectRootDir / "Assets";

		struct PendingAsset
		{
			std::string guid;
			uint32_t type;
			fs::path filePath;
		};

		std::vector<PendingAsset> pendingAssets;

		// 1. Добавляем манифест project.json под фиксированным GUID
		pendingAssets.push_back({
			"00000000-0000-0000-0000-000000000000",
			static_cast<uint32_t>(zzz::package::AssetType::ProjectManifest),
			projPath
		});

		// 2. Сканируем папки сцен, вьюшек и ассетов для добавления в package.dat
		if (fs::exists(assetsDir))
		{
			for (const auto& entry : fs::recursive_directory_iterator(assetsDir))
			{
				if (!entry.is_regular_file())
					continue;

				fs::path path = entry.path();
				std::string ext = path.extension().string();

				if (ext == ".meta" || ext == ".cpp")
					continue;

				// Ищем соответствующий .meta файл
				fs::path metaPath = path.string() + ".meta";
				std::string guid = "unknown";
				uint32_t typeVal = static_cast<uint32_t>(zzz::package::AssetType::BinaryAsset);

				if (ext == ".zs")
					typeVal = static_cast<uint32_t>(zzz::package::AssetType::Scene);
				else if (ext == ".zv")
					typeVal = static_cast<uint32_t>(zzz::package::AssetType::View);
				else if (ext == ".h" || ext == ".hpp")
					typeVal = static_cast<uint32_t>(zzz::package::AssetType::Script);

				if (fs::exists(metaPath))
				{
					std::ifstream metaFile(metaPath);
					std::string line;
					while (std::getline(metaFile, line))
					{
						auto pos = line.find("\"guid\"");
						if (pos != std::string::npos)
						{
							auto valStart = line.find('"', pos + 6);
							if (valStart != std::string::npos)
							{
								auto valEnd = line.find('"', valStart + 1);
								if (valEnd != std::string::npos)
								{
									guid = line.substr(valStart + 1, valEnd - valStart - 1);
									break;
								}
							}
						}
					}
				}

				if (guid != "unknown" && !guid.empty())
				{
					pendingAssets.push_back({ guid, typeVal, path });
				}
			}
		}

		// 3. Формируем бинарный пакет package.dat
		fs::create_directories(outPath.parent_path());
		std::ofstream outFile(outPath, std::ios::binary);
		if (!outFile.is_open())
			return false;

		zzz::package::PackageHeader header{};
		header.entryCount = static_cast<uint32_t>(pendingAssets.size());

		uint64_t headerSize = sizeof(zzz::package::PackageHeader);
		uint64_t indexTableSize = pendingAssets.size() * sizeof(zzz::package::PackageEntry);
		uint64_t currentOffset = headerSize + indexTableSize;

		std::vector<zzz::package::PackageEntry> indexTable;
		std::vector<std::vector<char>> payloads;

		for (const auto& item : pendingAssets)
		{
			std::ifstream inFile(item.filePath, std::ios::binary | std::ios::ate);
			if (!inFile.is_open())
				continue;

			uint64_t fileSize = static_cast<uint64_t>(inFile.tellg());
			inFile.seekg(0, std::ios::beg);

			std::vector<char> buffer(fileSize);
			inFile.read(buffer.data(), fileSize);

			zzz::package::PackageEntry entry{};
			std::strncpy(entry.guid, item.guid.c_str(), sizeof(entry.guid) - 1);
			entry.assetType = item.type;
			entry.offset = currentOffset;
			entry.size = fileSize;

			indexTable.push_back(entry);
			payloads.push_back(std::move(buffer));

			currentOffset += fileSize;
		}

		// Обновляем количество элементов
		header.entryCount = static_cast<uint32_t>(indexTable.size());

		// Записываем заголовок
		outFile.write(reinterpret_cast<const char*>(&header), sizeof(zzz::package::PackageHeader));

		// Записываем таблицу индексов
		for (const auto& entry : indexTable)
		{
			outFile.write(reinterpret_cast<const char*>(&entry), sizeof(zzz::package::PackageEntry));
		}

		// Записываем блоки данных
		for (const auto& payload : payloads)
		{
			outFile.write(payload.data(), payload.size());
		}

		return true;
	}
}
