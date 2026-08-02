#include "PackagePacker.h"
#include <fstream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <json.hpp>

#include <logger/logger.h>
#include <core/Enums/ePackage.h>
#include <core/IO/GamePackage/PackageHeader.h>
#include <core/IO/GamePackage/PackageEntry.h>
#include <core/IO/GamePackage/ProjectManifestData.h>
#include <core/IO/GamePackage/SceneData.h>
#include <core/IO/GamePackage/ViewData.h>
#include <core/IO/GamePackage/PrefabData.h>
#include <core/Constants.h>

namespace zzz::builder
{
	namespace fs = std::filesystem;
	using json = nlohmann::json;

	struct PendingAsset
	{
		std::string name;
		std::string guid;
		uint32_t type;
		fs::path filePath;
	};

	static std::vector<std::byte> SerializeAssetToBinary(const PendingAsset& item)
	{
		Serializer serializer;
		std::vector<std::byte> result;

		try
		{
			std::ifstream inFile(item.filePath);
			if (!inFile.is_open())
				return {};

			json root = json::parse(inFile, nullptr, false);
			if (root.is_discarded())
			{
				// Если это невалидный JSON, читаем как обычный бинарник
				std::ifstream rawFile(item.filePath, std::ios::binary | std::ios::ate);
				if (!rawFile.is_open()) return {};
				uint64_t size = static_cast<uint64_t>(rawFile.tellg());
				rawFile.seekg(0, std::ios::beg);
				result.resize(size);
				rawFile.read(reinterpret_cast<char*>(result.data()), size);
				return result;
			}

			auto assetType = static_cast<zzz::common::ePackage>(item.type);

			if (assetType == zzz::common::ePackage::ProjectManifest)
			{
				std::vector<Guid> gameScriptGuids;
				if (root.contains("game_scripts") && root["game_scripts"].is_array())
				{
					for (const auto& elem : root["game_scripts"])
					{
						if (elem.is_string())
						{
							if (auto parsed = Guid::Parse(elem.get<std::string>()))
								gameScriptGuids.push_back(*parsed);
						}
					}
				}
				else if (root.contains("game_script") && root["game_script"].is_string())
				{
					if (auto parsed = Guid::Parse(root["game_script"].get<std::string>()))
						gameScriptGuids.push_back(*parsed);
				}

				std::vector<Guid> sceneGuids;
				if (root.contains("scenes") && root["scenes"].is_array())
				{
					for (const auto& elem : root["scenes"])
					{
						if (elem.is_string())
						{
							if (auto parsed = Guid::Parse(elem.get<std::string>()))
								sceneGuids.push_back(*parsed);
						}
					}
				}

				std::vector<Guid> viewGuids;
				if (root.contains("views") && root["views"].is_array())
				{
					for (const auto& elem : root["views"])
					{
						if (elem.is_string())
						{
							if (auto parsed = Guid::Parse(elem.get<std::string>()))
								viewGuids.push_back(*parsed);
						}
					}
				}

				Guid startViewGuid{};
				if (root.contains("start_view") && root["start_view"].is_string())
				{
					if (auto parsed = Guid::Parse(root["start_view"].get<std::string>()))
						startViewGuid = *parsed;
				}
				else if (root.contains("startView") && root["startView"].is_string())
				{
					if (auto parsed = Guid::Parse(root["startView"].get<std::string>()))
						startViewGuid = *parsed;
				}

				zzz::core::ProjectManifestData manifestData(startViewGuid, gameScriptGuids, sceneGuids, viewGuids);
				serializer.Serialize(result, manifestData);
			}
			else if (assetType == zzz::common::ePackage::Scene)
			{
				std::vector<Guid> sceneScriptGuids;
				if (root.contains("scripts") && root["scripts"].is_array())
				{
					for (const auto& elem : root["scripts"])
					{
						if (elem.is_string())
						{
							if (auto parsed = Guid::Parse(elem.get<std::string>()))
								sceneScriptGuids.push_back(*parsed);
						}
					}
				}
				else if (root.contains("script") && root["script"].is_string())
				{
					if (auto parsed = Guid::Parse(root["script"].get<std::string>()))
						sceneScriptGuids.push_back(*parsed);
				}

				zzz::core::SceneData sceneData(sceneScriptGuids);
				serializer.Serialize(result, sceneData);
			}
			else if (assetType == zzz::common::ePackage::View)
			{
				zU32 width = root.value("width", 800u);
				zU32 height = root.value("height", 600u);
				bool isActive = root.value("is_active", root.value("active", true));

				Guid sceneGuid{};
				if (root.contains("scene") && root["scene"].is_string())
				{
					if (auto parsed = Guid::Parse(root["scene"].get<std::string>()))
						sceneGuid = *parsed;
				}

				std::vector<Guid> uiScriptGuids;
				if (root.contains("scripts") && root["scripts"].is_array())
				{
					for (const auto& elem : root["scripts"])
					{
						if (elem.is_string())
						{
							if (auto parsed = Guid::Parse(elem.get<std::string>()))
								uiScriptGuids.push_back(*parsed);
						}
					}
				}

				zzz::core::ViewData viewData(Size2D<zU32>{ width, height }, sceneGuid, uiScriptGuids, isActive);
				serializer.Serialize(result, viewData);
			}
			else if (assetType == zzz::common::ePackage::Prefab)
			{
				zzz::core::PrefabData prefabData;
				serializer.Serialize(result, prefabData);
			}
			else
			{
				std::ifstream rawFile(item.filePath, std::ios::binary | std::ios::ate);
				if (!rawFile.is_open()) return {};
				uint64_t size = static_cast<uint64_t>(rawFile.tellg());
				rawFile.seekg(0, std::ios::beg);
				result.resize(size);
				rawFile.read(reinterpret_cast<char*>(result.data()), size);
			}
		}
		catch (...)
		{
			// При исключении считываем исходные бинарные байты
			std::ifstream rawFile(item.filePath, std::ios::binary | std::ios::ate);
			if (!rawFile.is_open()) return {};
			uint64_t size = static_cast<uint64_t>(rawFile.tellg());
			rawFile.seekg(0, std::ios::beg);
			result.resize(size);
			rawFile.read(reinterpret_cast<char*>(result.data()), size);
		}

		return result;
	}

	bool PackagePacker::PackProject(const fs::path& sourceDir, const fs::path& destinationDir)
	{
		fs::path outPath = destinationDir / zzz::common::c_GamePackageFileName;
		std::vector<PendingAsset> pendingAssets;

		// 1. Упаковка project.json под служебным GUID манифеста
		fs::path projJsonPath = sourceDir / "project.json";
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
					continue;
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

		std::vector<std::vector<std::byte>> payloads;
		std::vector<uint64_t> fileSizes;

		for (const auto& item : pendingAssets)
		{
			std::vector<std::byte> payload = SerializeAssetToBinary(item);
			uint64_t payloadSize = payload.size();

			payloads.push_back(std::move(payload));
			fileSizes.push_back(payloadSize);
		}

		// Пасс 1: Измеряем размер serialized header + entries
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
				0,
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

		// Записываем бинарные блоки данных
		for (const auto& payload : payloads)
		{
			if (!payload.empty())
				outFile.write(reinterpret_cast<const char*>(payload.data()), payload.size());
		}

		return true;
	}
}
