#include "PackagePacker.h"
#include <fstream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <core/enums/ePackage.h>
#include <core/io/gamepackage/PackageHeader.h>
#include <core/io/gamepackage/PackageEntry.h>
#include <core/constants.h>

namespace zzz::builder
{
	namespace fs = std::filesystem;

	struct PendingAsset
	{
		std::string name;
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
				"ProjectManifest",
				"00000000-0000-0000-0000-000000000001",
				static_cast<uint32_t>(zzz::common::ePackage::ProjectManifest),
				projJsonPath
			});
		}

		// 2. Поиск сцен (*.zs), вьюх (*.zv) и префабов (*.zp) в исходной директории
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
				else if (ext == ".zp")
				{
					typeVal = static_cast<uint32_t>(zzz::common::ePackage::Prefab);
				}
				else
				{
					continue; // Упаковываем сцены, вьюхи, префабы и манифест
				}

				fs::path path = entry.path();
				std::string assetName = path.stem().string();
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
					pendingAssets.push_back({ assetName, guid, typeVal, path });
				}
			}
		}

		// 3. Формирование бинарного файла package.dat в подпапке destinationDir/assets/
		fs::create_directories(outPath.parent_path());
		std::ofstream outFile(outPath, std::ios::binary);
		if (!outFile.is_open())
			return false;

		std::vector<std::vector<char>> payloads;
		std::vector<uint64_t> fileSizes;

		for (const auto& item : pendingAssets)
		{
			std::ifstream inFile(item.filePath, std::ios::binary | std::ios::ate);
			if (!inFile.is_open())
			{
				payloads.push_back({});
				fileSizes.push_back(0);
				continue;
			}

			uint64_t fileSize = static_cast<uint64_t>(inFile.tellg());
			inFile.seekg(0, std::ios::beg);

			std::vector<char> buffer(fileSize);
			inFile.read(buffer.data(), fileSize);

			payloads.push_back(std::move(buffer));
			fileSizes.push_back(fileSize);
		}

		// Пасс 1: Создаем предварительный список PackageEntry и измеряем размер serialized header + entries.
		Serializer serializer;
		std::vector<zzz::core::PackageEntry> dummyEntries;
		dummyEntries.reserve(pendingAssets.size());

		for (size_t i = 0; i < pendingAssets.size(); ++i)
		{
			const auto& item = pendingAssets[i];
			auto parsedGuid = Guid::Parse(item.guid);
			dummyEntries.emplace_back(
				item.name,
				parsedGuid ? *parsedGuid : Guid{},
				item.type,
				0, // Смещение измерим далее
				fileSizes[i]
			);
		}

		zzz::core::PackageHeader dummyHeader(
			c_GamePackageHeader,
			Version{ c_GamePackageFileMajorVersion, c_GamePackageFileMinorVersion, c_GamePackageFilePatchVersion },
			static_cast<uint32_t>(dummyEntries.size())
		);

		std::vector<std::byte> headerBuffer;
		if (!serializer.Serialize(headerBuffer, dummyHeader))
			return false;

		for (const auto& entry : dummyEntries)
		{
			if (!serializer.Serialize(headerBuffer, entry))
				return false;
		}

		const uint64_t initialOffset = headerBuffer.size();

		// Пасс 2: Строим итоговые записи с правильными offset
		std::vector<zzz::core::PackageEntry> finalEntries;
		finalEntries.reserve(pendingAssets.size());
		uint64_t currentOffset = initialOffset;

		for (size_t i = 0; i < pendingAssets.size(); ++i)
		{
			const auto& item = pendingAssets[i];
			auto parsedGuid = Guid::Parse(item.guid);
			finalEntries.emplace_back(
				item.name,
				parsedGuid ? *parsedGuid : Guid{},
				item.type,
				currentOffset,
				fileSizes[i]
			);

			currentOffset += fileSizes[i];
		}

		// Записываем финальный headerBuffer
		headerBuffer.clear();
		if (!serializer.Serialize(headerBuffer, dummyHeader))
			return false;

		for (const auto& entry : finalEntries)
		{
			if (!serializer.Serialize(headerBuffer, entry))
				return false;
		}

		// Записываем сериализованный заголовок и таблицу в файл
		outFile.write(reinterpret_cast<const char*>(headerBuffer.data()), headerBuffer.size());

		// Записываем блоки данных
		for (const auto& payload : payloads)
		{
			if (!payload.empty())
				outFile.write(payload.data(), payload.size());
		}

		return true;
	}
}
