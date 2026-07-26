#include "PackagePacker.h"
#include <fstream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <common/package_format.h>
#include <common/constants.h>

namespace zzz::builder
{
	namespace fs = std::filesystem;

	struct PendingAsset
	{
		std::string guid;
		uint32_t type;
		fs::path filePath;
	};

	bool PackagePacker::PackProject(const fs::path& sourceDir, const fs::path& destinationDir)
	{
		fs::path outPath = destinationDir / "assets" / zzz::common::c_GamePackageFileName;
		std::vector<PendingAsset> pendingAssets;

		// 1. Упаковка project.json под служебным GUID манифеста
		fs::path projJsonPath = sourceDir / zzz::common::c_ProjectJsonFileName;
		if (fs::exists(projJsonPath))
		{
			pendingAssets.push_back({
				"00000000-0000-0000-0000-000000000001",
				static_cast<uint32_t>(zzz::package::AssetType::ProjectManifest),
				projJsonPath
			});
		}

		// 2. Поиск сцен (*.zs) и вьюх (*.zv) в исходной директории
		if (fs::exists(sourceDir))
		{
			for (const auto& entry : fs::recursive_directory_iterator(sourceDir))
			{
				if (!entry.is_regular_file())
					continue;

				auto ext = entry.path().extension().string();
				uint32_t typeVal = 0;

				if (ext == ".zs")
				{
					typeVal = static_cast<uint32_t>(zzz::package::AssetType::Scene);
				}
				else if (ext == ".zv")
				{
					typeVal = static_cast<uint32_t>(zzz::package::AssetType::View);
				}
				else
				{
					continue; // На данном этапе упаковываем только сцены, вьюхи и манифест
				}

				fs::path path = entry.path();
				fs::path metaPath = path.string() + ".meta";
				std::string guid = "unknown";

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

		// 3. Формирование бинарного файла package.dat в подпапке destinationDir/assets/
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
			const auto copyLen = (std::min)(item.guid.size(), sizeof(entry.guid) - 1);
			std::memcpy(entry.guid, item.guid.data(), copyLen);
			entry.guid[copyLen] = '\0';
			entry.assetType = item.type;
			entry.offset = currentOffset;
			entry.size = fileSize;

			indexTable.push_back(entry);
			payloads.push_back(std::move(buffer));

			currentOffset += fileSize;
		}

		// Записываем заголовок
		header.entryCount = static_cast<uint32_t>(indexTable.size());
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
