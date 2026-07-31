#include "PackagePacker.h"
#include <fstream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <common/enums/ePackage.h>
#include <common/package/PackageHeader.h>
#include <common/package/PackageEntry.h>
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
		fs::path outPath = destinationDir / zzz::common::c_GamePackageFileName;
		std::vector<PendingAsset> pendingAssets;

		// 1. Упаковка project.json под служебным GUID манифеста
		fs::path projJsonPath = sourceDir / zzz::common::c_ProjectJsonFileName;
		if (fs::exists(projJsonPath))
		{
			pendingAssets.push_back({
				"00000000-0000-0000-0000-000000000001",
				static_cast<uint32_t>(zzz::common::ePackage::ProjectManifest),
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
					typeVal = static_cast<uint32_t>(zzz::common::ePackage::Scene);
				}
				else if (ext == ".zv")
				{
					typeVal = static_cast<uint32_t>(zzz::common::ePackage::View);
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

		// Сначала вычисляем смещение первого payload.
		// Размер сериализованного заголовка: 3 (magic) + 12 (version) + 4 (entryCount) = 19 байт.
		// Размер каждой сериализованной записи PackageEntry: 16 (guid) + 4 (assetType) + 8 (offset) + 8 (size) = 36 байт.
		constexpr uint64_t headerSize = 19;
		constexpr uint64_t entrySize = 36;
		uint64_t currentOffset = headerSize + (pendingAssets.size() * entrySize);

		std::vector<zzz::core::PackageEntry> indexTable;
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

			auto parsedGuid = Guid::Parse(item.guid);
			zzz::core::PackageEntry entry(
				parsedGuid ? *parsedGuid : Guid{},
				item.type,
				currentOffset,
				fileSize
			);

			indexTable.push_back(entry);
			payloads.push_back(std::move(buffer));

			currentOffset += fileSize;
		}

		zzz::core::PackageHeader header(
			c_GamePackageHeader,
			Version{ c_GamePackageFileMajorVersion, c_GamePackageFileMinorVersion, c_GamePackageFilePatchVersion },
			static_cast<uint32_t>(indexTable.size())
		);

		// Сериализуем заголовок и таблицу индексов в буфер байт с помощью Serializer
		Serializer serializer;
		std::vector<std::byte> headerBuffer;
		if (!serializer.Serialize(headerBuffer, header))
			return false;

		for (const auto& entry : indexTable)
		{
			if (!serializer.Serialize(headerBuffer, entry))
				return false;
		}

		// Записываем сериализованный заголовок и таблицу в файл
		outFile.write(reinterpret_cast<const char*>(headerBuffer.data()), headerBuffer.size());

		// Записываем блоки данных
		for (const auto& payload : payloads)
		{
			outFile.write(payload.data(), payload.size());
		}

		return true;
	}
}
