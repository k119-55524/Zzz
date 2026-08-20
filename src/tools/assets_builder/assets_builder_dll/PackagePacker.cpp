#include "PackagePacker.h"
#include <fstream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <json.hpp>

#include <logger/logger.h>
#include <core/Core.h>
#include <core/IO/package/PrimaryViewData.h>
#include <core/IO/package/PackageEntry.h>
#include <core/IO/package/PackageHeader.h>
#include <core/IO/package/PrefabData.h>
#include <core/IO/package/ProjectManifestData.h>
#include <core/IO/package/platforms/project/ProjectPlatformDataAndroid.h>
#include <core/IO/package/platforms/project/ProjectPlatformDataLinux.h>
#include <core/IO/package/platforms/project/ProjectPlatformDataMacOS.h>
#include <core/IO/package/platforms/project/ProjectPlatformDataMSWin.h>
#include <core/IO/package/platforms/project/ProjectPlatformDataiOS.h>
#include <core/IO/package/SceneData.h>
#include <core/IO/package/ViewData.h>
#include <core/IO/package/platforms/start_view/ViewDataAndroid.h>
#include <core/IO/package/platforms/start_view/ViewDataLinux.h>
#include <core/IO/package/platforms/start_view/ViewDataMacOS.h>
#include <core/IO/package/platforms/start_view/ViewDataMSWin.h>
#include <core/IO/package/platforms/start_view/ViewDataiOS.h>

namespace zzz::builder
{
	namespace fs = std::filesystem;
	using json = nlohmann::json;
	using namespace zzz::core;

	struct PendingAsset
	{
		std::string name;
		std::string guid;
		uint32_t type;
		fs::path filePath;
	};

	static std::string ToPlatformString(zzz::core::eTargetPlatform targetPlatform)
	{
		switch (targetPlatform)
		{
		case zzz::core::eTargetPlatform::Windows: return "Windows";
		case zzz::core::eTargetPlatform::Linux: return "Linux";
		case zzz::core::eTargetPlatform::Android: return "Android";
		case zzz::core::eTargetPlatform::MacOS: return "MacOS";
		case zzz::core::eTargetPlatform::iOS: return "iOS";
		}

		return "Windows";
	}

	static json ResolvePlatformJson(const json& root, const fs::path& projectDir, zzz::core::eTargetPlatform targetPlatform)
	{
		const std::string platformName = ToPlatformString(targetPlatform);
		json platformRoot = root.contains("platform") && root["platform"].is_object()
			? root["platform"]
			: json::object();

		if (root.contains("platform_configs") && root["platform_configs"].is_array())
		{
			for (const auto& configEntry : root["platform_configs"])
			{
				if (!configEntry.is_object())
					continue;

				if (configEntry.value("platform", "") != platformName)
					continue;

				std::string configFileName = configEntry.value("file", "");
				if (configFileName.empty())
					break;

				fs::path configPath = projectDir / configFileName;
				std::ifstream configFile(configPath);
				if (!configFile.is_open())
					break;

				json configRoot = json::parse(configFile, nullptr, false);
				if (!configRoot.is_discarded() && configRoot.contains("platform") && configRoot["platform"].is_object())
				{
					for (const auto& [key, value] : configRoot["platform"].items())
					{
						platformRoot[key] = value;
					}
				}

				break;
			}
		}

		return platformRoot;
	}

	static std::expected<void, std::string> SerializeProjectPlatformData(std::vector<std::byte>& result, const Serializer& serializer, const json& platformRoot, zzz::core::eTargetPlatform targetPlatform)
	{
		switch (targetPlatform)
		{
		case zzz::core::eTargetPlatform::Windows:
		{
			ProjectPlatformDataMSWin winData(platformRoot.value("windowClassName", "ZzzEngineWindowClass"));
			return serializer.Serialize(result, winData);
		}
		case zzz::core::eTargetPlatform::Linux:
		{
			ProjectPlatformDataLinux linuxData;
			return serializer.Serialize(result, linuxData);
		}
		case zzz::core::eTargetPlatform::Android:
		{
			ProjectPlatformDataAndroid androidData;
			return serializer.Serialize(result, androidData);
		}
		case zzz::core::eTargetPlatform::MacOS:
		{
			ProjectPlatformDataMacOS macData;
			return serializer.Serialize(result, macData);
		}
		case zzz::core::eTargetPlatform::iOS:
		{
			ProjectPlatformDataiOS iosData;
			return serializer.Serialize(result, iosData);
		}
		}

		ProjectPlatformDataMSWin winData(platformRoot.value("windowClassName", "ZzzEngineWindowClass"));
		return serializer.Serialize(result, winData);
	}

	static json ResolveStartViewJson(const json& root, const fs::path& projectDir, zzz::core::eTargetPlatform targetPlatform)
	{
		const std::string platformName = ToPlatformString(targetPlatform);
		json startViewRoot = root.contains("startView") && root["startView"].is_object()
			? root["startView"]
			: root;
		json configRoot = root;

		if (!configRoot.contains("platform_configs"))
		{
			std::ifstream projectFile(projectDir / "project.json");
			if (projectFile.is_open())
			{
				json projectRoot = json::parse(projectFile, nullptr, false);
				if (!projectRoot.is_discarded())
					configRoot = std::move(projectRoot);
			}
		}

		if (configRoot.contains("platform_configs") && configRoot["platform_configs"].is_array())
		{
			for (const auto& configEntry : configRoot["platform_configs"])
			{
				if (!configEntry.is_object())
					continue;

				if (configEntry.value("platform", "") != platformName)
					continue;

				std::string configFileName = configEntry.value("file", "");
				if (configFileName.empty())
					break;

				fs::path configPath = projectDir / configFileName;
				std::ifstream configFile(configPath);
				if (!configFile.is_open())
					break;

				json platformRoot = json::parse(configFile, nullptr, false);
				if (!platformRoot.is_discarded() && platformRoot.contains("startView") && platformRoot["startView"].is_object())
				{
					for (const auto& [key, value] : platformRoot["startView"].items())
					{
						startViewRoot[key] = value;
					}
					return startViewRoot;
				}

				break;
			}
		}

		return startViewRoot;
	}

	static Size2D<zU32> ReadSize(const json& root, zU32 defaultWidth = 1280, zU32 defaultHeight = 720)
	{
		const auto defaultSize = root.value("defaultSize", json::object());
		return Size2D<zU32>{
			root.value("width", defaultSize.value("width", defaultWidth)),
			root.value("height", defaultSize.value("height", defaultHeight))
		};
	}

	static eAndroidScreenOrientation ReadAndroidOrientation(std::string_view value)
	{
		if (value == "Sensor") return eAndroidScreenOrientation::Sensor;
		if (value == "Portrait") return eAndroidScreenOrientation::Portrait;
		if (value == "LandscapeRight") return eAndroidScreenOrientation::LandscapeRight;
		return eAndroidScreenOrientation::LandscapeLeft;
	}

	static eAndroidCutoutMode ReadAndroidCutoutMode(std::string_view value)
	{
		if (value == "Default") return eAndroidCutoutMode::Default;
		if (value == "Never") return eAndroidCutoutMode::Never;
		return eAndroidCutoutMode::ShortEdges;
	}

	static eLinuxWindowMode ReadLinuxWindowMode(std::string_view value)
	{
		if (value == "Fullscreen") return eLinuxWindowMode::Fullscreen;
		return eLinuxWindowMode::Windowed;
	}

	static eLinuxDisplayServer ReadLinuxDisplayServer(std::string_view value)
	{
		if (value == "Wayland") return eLinuxDisplayServer::Wayland;
		if (value == "X11") return eLinuxDisplayServer::X11;
		return eLinuxDisplayServer::Auto;
	}

	static eMacOSWindowMode ReadMacOSWindowMode(std::string_view value)
	{
		if (value == "Fullscreen") return eMacOSWindowMode::Fullscreen;
		return eMacOSWindowMode::Windowed;
	}

	static eiOSScreenOrientation ReadiOSOrientation(std::string_view value)
	{
		if (value == "AutoRotate") return eiOSScreenOrientation::AutoRotate;
		if (value == "Portrait") return eiOSScreenOrientation::Portrait;
		if (value == "LandscapeRight") return eiOSScreenOrientation::LandscapeRight;
		return eiOSScreenOrientation::LandscapeLeft;
	}

	static eiOSSafeAreaMode ReadiOSSafeAreaMode(std::string_view value)
	{
		if (value == "UseSafeArea") return eiOSSafeAreaMode::UseSafeArea;
		return eiOSSafeAreaMode::ExtendIntoSafeArea;
	}

	static eiOSHomeIndicatorMode ReadiOSHomeIndicatorMode(std::string_view value)
	{
		if (value == "Visible") return eiOSHomeIndicatorMode::Visible;
		return eiOSHomeIndicatorMode::AutoHidden;
	}

	static eMSWinWindowMode ReadMSWinWindowMode(const json& root)
	{
		const std::string windowMode = root.value("windowMode", "");
		if (windowMode == "BorderlessFullscreen") return eMSWinWindowMode::BorderlessFullscreen;
		if (windowMode == "ExclusiveFullscreen") return eMSWinWindowMode::ExclusiveFullscreen;
		if (root.value("fullscreen", false)) return eMSWinWindowMode::BorderlessFullscreen;
		return eMSWinWindowMode::Windowed;
	}

	static std::vector<std::byte> SerializeAssetToBinary(const PendingAsset& item, const fs::path& projectDir, zzz::core::eTargetPlatform targetPlatform)
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

			auto assetType = static_cast<zzz::core::ePackage>(item.type);

			if (assetType == zzz::core::ePackage::ProjectManifest)
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

				json platformRoot = ResolvePlatformJson(root, projectDir, targetPlatform);
				zU32 maxQueueSize = c_MaxNetworkLogQueueSize;
				zU16 loggerPort = c_DefaultLoggerPort;

				json loggerRoot = root.contains("logger") && root["logger"].is_object()
					? root["logger"]
					: (platformRoot.contains("logger") && platformRoot["logger"].is_object() ? platformRoot["logger"] : json::object());

				if (loggerRoot.contains("maxQueueSize") && loggerRoot["maxQueueSize"].is_number_integer())
					maxQueueSize = loggerRoot["maxQueueSize"].get<zU32>();
				if (loggerRoot.contains("port") && loggerRoot["port"].is_number_integer())
					loggerPort = loggerRoot["port"].get<zU16>();

				const zU32 scriptsCount = static_cast<zU32>(gameScriptGuids.size());
				if (auto res = serializer.Serialize(result, scriptsCount); !res) return {};
				for (const auto& guid : gameScriptGuids)
				{
					if (auto res = serializer.Serialize(result, guid); !res) return {};
				}

				const zU32 scenesCount = static_cast<zU32>(sceneGuids.size());
				if (auto res = serializer.Serialize(result, scenesCount); !res) return {};
				for (const auto& guid : sceneGuids)
				{
					if (auto res = serializer.Serialize(result, guid); !res) return {};
				}

				const zU32 viewsCount = static_cast<zU32>(viewGuids.size());
				if (auto res = serializer.Serialize(result, viewsCount); !res) return {};
				for (const auto& guid : viewGuids)
				{
					if (auto res = serializer.Serialize(result, guid); !res) return {};
				}

				if (auto res = serializer.Serialize(result, maxQueueSize); !res) return {};
				if (auto res = serializer.Serialize(result, loggerPort); !res) return {};

				if (auto res = SerializeProjectPlatformData(result, serializer, platformRoot, targetPlatform); !res)
					return {};
			}
			else if (assetType == zzz::core::ePackage::PrimaryView)
			{
				json startViewRoot = ResolveStartViewJson(root, projectDir, targetPlatform);

				Guid sceneGuid{};
				if (startViewRoot.contains("scene") && startViewRoot["scene"].is_string())
				{
					if (auto parsed = Guid::Parse(startViewRoot["scene"].get<std::string>()))
						sceneGuid = *parsed;
				}

				std::vector<Guid> uiScriptGuids;
				if (startViewRoot.contains("scripts") && startViewRoot["scripts"].is_array())
				{
					for (const auto& elem : startViewRoot["scripts"])
					{
						if (elem.is_string())
						{
							if (auto parsed = Guid::Parse(elem.get<std::string>()))
								uiScriptGuids.push_back(*parsed);
						}
					}
				}

				Guid viewGuid{};
				if (auto parsed = Guid::Parse(item.guid))
					viewGuid = *parsed;

				// Сначала сериализуем общие поля PrimaryViewData (ViewGuid, SceneGuid, ScriptGuids)
				if (auto res = serializer.Serialize(result, viewGuid); !res) return {};
				if (auto res = serializer.Serialize(result, sceneGuid); !res) return {};
				zU32 scriptsCount = static_cast<zU32>(uiScriptGuids.size());
				if (auto res = serializer.Serialize(result, scriptsCount); !res) return {};
				for (const auto& guid : uiScriptGuids)
				{
					if (auto res = serializer.Serialize(result, guid); !res) return {};
				}

				// Сериализация платформенно-зависимых данных
				switch (targetPlatform)
				{
				case zzz::core::eTargetPlatform::Android:
				{
					const auto orient = ReadAndroidOrientation(startViewRoot.value("orientation", "LandscapeLeft"));
					zU32 fps = startViewRoot.value("targetFPS", 60u);
					const auto cutout = ReadAndroidCutoutMode(startViewRoot.value("cutoutMode", "ShortEdges"));
					bool keepOn = startViewRoot.value("keepScreenOn", true);

					ViewDataAndroid androidData(orient, fps, cutout, keepOn);
					if (auto res = serializer.Serialize(result, androidData); !res) return {};
					break;
				}
				case zzz::core::eTargetPlatform::Linux:
				{
					std::string title = startViewRoot.value("title", "Game Window");
					auto size = ReadSize(startViewRoot);
					auto windowMode = ReadLinuxWindowMode(startViewRoot.value("windowMode", "Windowed"));
					bool resizable = startViewRoot.value("resizable", true);
					auto displayServer = ReadLinuxDisplayServer(startViewRoot.value("displayServer", "Auto"));

					ViewDataLinux linuxData(title, size, windowMode, resizable, displayServer);
					if (auto res = serializer.Serialize(result, linuxData); !res) return {};
					break;
				}
				case zzz::core::eTargetPlatform::MacOS:
				{
					std::string title = startViewRoot.value("title", "Game Window");
					auto size = ReadSize(startViewRoot);
					auto windowMode = ReadMacOSWindowMode(startViewRoot.value("windowMode", "Windowed"));
					bool resizable = startViewRoot.value("resizable", true);

					ViewDataMacOS macData(title, size, windowMode, resizable);
					if (auto res = serializer.Serialize(result, macData); !res) return {};
					break;
				}
				case zzz::core::eTargetPlatform::iOS:
				{
					const auto orient = ReadiOSOrientation(startViewRoot.value("orientation", "LandscapeLeft"));
					const auto safeAreaMode = ReadiOSSafeAreaMode(startViewRoot.value("safeAreaMode", "ExtendIntoSafeArea"));
					const auto homeIndicatorMode = ReadiOSHomeIndicatorMode(startViewRoot.value("homeIndicatorMode", "AutoHidden"));

					ViewDataiOS iosData(orient, safeAreaMode, homeIndicatorMode);
					if (auto res = serializer.Serialize(result, iosData); !res) return {};
					break;
				}
				case zzz::core::eTargetPlatform::Windows:
				default:
				{
					std::string title = startViewRoot.value("title", "Game Window");
					auto size = ReadSize(startViewRoot);
					auto windowMode = ReadMSWinWindowMode(startViewRoot);
					bool resizable = startViewRoot.value("resizable", true);

					ViewDataMSWin winData(title, size, windowMode, resizable);
					if (auto res = serializer.Serialize(result, winData); !res) return {};
					break;
				}
				}
			}
			else if (assetType == zzz::core::ePackage::Scene)
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
				if (auto res = serializer.Serialize(result, sceneData); !res)
					return {};
			}
			else if (assetType == zzz::core::ePackage::View)
			{
				zU32 width = root.value("width", 800u);
				zU32 height = root.value("height", 600u);

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

				zzz::core::ViewData viewData(Size2D<zU32>{ width, height }, sceneGuid, uiScriptGuids);
				if (auto res = serializer.Serialize(result, viewData); !res)
					return {};
			}
			else if (assetType == zzz::core::ePackage::Prefab)
			{
				zzz::core::PrefabData prefabData;
				if (auto res = serializer.Serialize(result, prefabData); !res)
					return {};
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

	bool PackagePacker::PackProject(const fs::path& sourceDir, const fs::path& destinationDir, zzz::core::eTargetPlatform targetPlatform)
	{
		fs::path outPath = destinationDir / zzz::core::c_GamePackageFileName;
		std::vector<PendingAsset> pendingAssets;

		// 1. Упаковка project.json под служебным GUID манифеста
		fs::path projJsonPath = sourceDir / "project.json";
		if (fs::exists(projJsonPath))
		{
			pendingAssets.push_back({
				"ProjectManifest",
				"00000000-0000-0000-0000-000000000001",
				static_cast<uint32_t>(zzz::core::ePackage::ProjectManifest),
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
					typeVal = static_cast<uint32_t>(zzz::core::ePackage::Scene);
				}
				else if (ext == ".zv")
				{
					typeVal = static_cast<uint32_t>(zzz::core::ePackage::View);
				}
				else if (ext == ".zp")
				{
					typeVal = static_cast<uint32_t>(zzz::core::ePackage::Prefab);
				}
				else if (ext == ".zav")
				{
					typeVal = static_cast<uint32_t>(zzz::core::ePackage::PrimaryView);
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

		bool hasPrimaryView = std::any_of(pendingAssets.begin(), pendingAssets.end(), [](const PendingAsset& item) {
			return item.type == static_cast<uint32_t>(zzz::core::ePackage::PrimaryView);
		});

		if (!hasPrimaryView)
		{
			pendingAssets.push_back({
				"MainPrimaryView",
				"00000000-0000-0000-0000-000000000002",
				static_cast<uint32_t>(zzz::core::ePackage::PrimaryView),
				projJsonPath
			});
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
			std::vector<std::byte> payload = SerializeAssetToBinary(item, sourceDir, targetPlatform);
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
