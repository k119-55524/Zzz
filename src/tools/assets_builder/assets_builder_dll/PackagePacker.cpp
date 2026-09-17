#include "PackagePacker.h"
#include <fstream>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <expected>
#include <vector>
#include <unordered_set>
#include <json.hpp>

#include <logger/logger.h>
#include <core/Core.h>
#include <core/utils/macros/LogMacros.h>
#include <core/IO/package/PrimaryViewData.h>
#include <core/IO/package/PackageEntry.h>
#include <core/io/DatFileHeader.h>
#include <core/IO/package/PrefabData.h>
#include <core/IO/package/ProjectManifestData.h>
#include <core/IO/package/platforms/project/ProjectPlatformDataAndroid.h>
#include <core/IO/package/platforms/project/ProjectPlatformDataLinux.h>
#include <core/IO/package/platforms/project/ProjectPlatformDataMacOS.h>
#include <core/IO/package/platforms/project/ProjectPlatformDataMSWin.h>
#include <core/IO/package/platforms/project/ProjectPlatformDataiOS.h>
#include <core/IO/package/SceneData.h>
#include <core/IO/package/ChildViewData.h>
#include <core/IO/package/IndependentViewData.h>
#include <core/IO/package/platforms/start_view/ViewDataAndroid.h>
#include <core/IO/package/platforms/start_view/ViewDataLinux.h>
#include <core/IO/package/platforms/start_view/ViewDataMacOS.h>
#include <core/IO/package/platforms/start_view/ViewDataMSWin.h>
#include <core/IO/package/platforms/start_view/ViewDataiOS.h>
#include <core/IO/package/MeshData.h>
#include <core/IO/package/GameObjectData.h>
#include <core/IO/ResourceStorageTraits.h>
#include <core/constants/PackageConstants.h>
#include <core/enums/eLayerType.h>
#include <core/IO/package/LayerData.h>
#include "AssetImportPipeline.h"
#include "AssetImporterRegistry.h"
#include "AssetScanner.h"
#include "ArchiveWriter.h"
#include "ProjectIdentityValidator.h"

Z_SET_LOG_CATEGORY(::zzz::core::Assets);

namespace zzz::builder
{
	namespace fs = std::filesystem;
	using json = nlohmann::json;
	using namespace zzz::core;

	struct PendingAsset
	{
		std::string name;
		Guid guid;
		uint32_t type;
		fs::path filePath;
	};

	struct PendingDataAsset
	{
		std::string name;
		Guid guid;
		eResourceType resourceType{ eResourceType::Unknown };
		std::vector<std::byte> payload;
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

	static json LoadPlatformConfigJson(const fs::path& projectDir, const std::string& platformConfigFile)
	{
		if (platformConfigFile.empty())
			return json::object();

		fs::path p(platformConfigFile);
		fs::path configPath = p.is_absolute() ? p : (projectDir / p);
		if (!fs::exists(configPath))
		{
			configPath = projectDir / "build_settings" / p;
		}

		std::ifstream configFile(configPath);
		if (!configFile.is_open())
			return json::object();

		json configRoot = json::parse(configFile, nullptr, false);
		if (configRoot.is_discarded())
			return json::object();

		return configRoot;
	}

	static json ResolvePlatformJson(const json& root, const fs::path& projectDir, zzz::core::eTargetPlatform /*targetPlatform*/, const std::string& platformConfigFile)
	{
		json platformRoot = root.contains("platform") && root["platform"].is_object()
			? root["platform"]
			: json::object();

		json configRoot = LoadPlatformConfigJson(projectDir, platformConfigFile);
		if (configRoot.contains("platform") && configRoot["platform"].is_object())
		{
			for (const auto& [key, value] : configRoot["platform"].items())
			{
				platformRoot[key] = value;
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

	static json ResolveStartViewJson(const json& root, const fs::path& projectDir, zzz::core::eTargetPlatform /*targetPlatform*/, const std::string& platformConfigFile)
	{
		json startViewRoot = root.contains("startView") && root["startView"].is_object()
			? root["startView"]
			: root;

		json configRoot = LoadPlatformConfigJson(projectDir, platformConfigFile);
		if (configRoot.contains("startView") && configRoot["startView"].is_object())
		{
			for (const auto& [key, value] : configRoot["startView"].items())
			{
				startViewRoot[key] = value;
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

	static zzz::engine::ClearConfig ReadClearConfig(const json& root)
	{
		zzz::engine::ClearConfig config;

		if (!root.contains("clear") || !root["clear"].is_object())
			return config;

		const auto& clearJson = root["clear"];

		// Surface mode
		std::string surfaceModeStr = clearJson.value("surfaceMode", clearJson.value("colorMode", "Color"));
		if (surfaceModeStr == "None") config.surface.mode = zzz::engine::eSurfaceClearMode::None;
		else if (surfaceModeStr == "Shader") config.surface.mode = zzz::engine::eSurfaceClearMode::Shader;
		else config.surface.mode = zzz::engine::eSurfaceClearMode::Color;

		// Color parsing
		if (clearJson.contains("color"))
		{
			const auto& c = clearJson["color"];
			if (c.is_string())
			{
				std::string colorName = c.get<std::string>();
				if (colorName == "CornflowerBlue") config.surface.color = zzz::math::Palette4::CornflowerBlue;
				else if (colorName == "Black") config.surface.color = zzz::math::Palette4::Black;
				else if (colorName == "White") config.surface.color = zzz::math::Palette4::White;
				else if (colorName == "Red") config.surface.color = zzz::math::Palette4::Red;
				else if (colorName == "Green") config.surface.color = zzz::math::Palette4::Green;
				else if (colorName == "Blue") config.surface.color = zzz::math::Palette4::Blue;
				else if (colorName == "Transparent") config.surface.color = zzz::math::Palette4::Transparent;
			}
			else if (c.is_array() && c.size() >= 3)
			{
				zF32 r = c[0].get<zF32>();
				zF32 g = c[1].get<zF32>();
				zF32 b = c[2].get<zF32>();
				zF32 a = c.size() >= 4 ? c[3].get<zF32>() : 1.0f;
				if (r > 1.0f || g > 1.0f || b > 1.0f)
				{
					zU8 aByte = (a > 1.0f) ? static_cast<zU8>(a) : static_cast<zU8>(a * 255.0f);
					config.surface.color = Color4<zU8>(static_cast<zU8>(r), static_cast<zU8>(g), static_cast<zU8>(b), aByte).ConvertTo<zF32>();
				}
				else
				{
					zF32 aNorm = (a > 1.0f) ? (a / 255.0f) : a;
					config.surface.color = zzz::math::Color4<zF32>(r, g, b, aNorm);
				}
			}
		}

		// Shader GUID
		if (clearJson.contains("shader") && clearJson["shader"].is_string())
		{
			config.surface.shaderGuid = Guid::Parse(clearJson["shader"].get<std::string>()).value_or(Guid{});
		}

		// Depth mode
		std::string depthModeStr = clearJson.value("depthMode", "Depth");
		if (depthModeStr == "None") config.depthBuffer.depthMode = zzz::engine::eClearDepthMode::None;
		else config.depthBuffer.depthMode = zzz::engine::eClearDepthMode::Depth;

		config.depthBuffer.depth = clearJson.value("depth", 1.0f);

		// Stencil mode
		std::string stencilModeStr = clearJson.value("stencilMode", "None");
		if (stencilModeStr == "Stencil") config.depthBuffer.stencilMode = zzz::engine::eClearStencilMode::Stencil;
		else config.depthBuffer.stencilMode = zzz::engine::eClearStencilMode::None;

		config.depthBuffer.stencil = static_cast<zU8>(clearJson.value("stencil", 0));

		return config;
	}

	static GameObjectData ParseGameObjectJson(const json& objJson, uint32_t parentIndex = 0xFFFFFFFF)
	{
		std::string name = objJson.value("name", "GameObject");

		Guid objGuid{};
		if (objJson.contains("guid") && objJson["guid"].is_string())
		{
			if (auto parsed = Guid::Parse(objJson["guid"].get<std::string>()))
				objGuid = *parsed;
		}
		if (objGuid == Guid{})
		{
			THROW_RUNTIME("GameObject '{}' не содержит обязательного валидного 'guid'.", name);
		}

		bool isEntity = objJson.value(c_FieldIsEntity, false);
		bool isActive = objJson.value("isActive", true);

		math::Vec3f position(0.0f, 0.0f, 0.0f);
		if (objJson.contains("position") && objJson["position"].is_array() && objJson["position"].size() >= 3)
		{
			position.x = objJson["position"][0].get<float>();
			position.y = objJson["position"][1].get<float>();
			position.z = objJson["position"][2].get<float>();
		}

		math::Quatf rotation(0.0f, 0.0f, 0.0f, 1.0f);
		if (objJson.contains("rotation") && objJson["rotation"].is_array() && objJson["rotation"].size() >= 4)
		{
			rotation.x = objJson["rotation"][0].get<float>();
			rotation.y = objJson["rotation"][1].get<float>();
			rotation.z = objJson["rotation"][2].get<float>();
			rotation.w = objJson["rotation"][3].get<float>();
		}

		math::Vec3f scale(1.0f, 1.0f, 1.0f);
		if (objJson.contains("scale") && objJson["scale"].is_array() && objJson["scale"].size() >= 3)
		{
			scale.x = objJson["scale"][0].get<float>();
			scale.y = objJson["scale"][1].get<float>();
			scale.z = objJson["scale"][2].get<float>();
		}

		std::vector<Guid> meshGuids;
		Guid materialGuid{};
		std::vector<Guid> materialGuids;

		if (objJson.contains("render"))
		{
			const auto& render = objJson["render"];
			if (render.is_array())
			{
				for (const auto& partElem : render)
				{
					if (partElem.is_object())
					{
						Guid smMesh{};
						Guid smMat{};
						if (partElem.contains("mesh") && partElem["mesh"].is_string())
						{
							if (auto parsed = Guid::Parse(partElem["mesh"].get<std::string>()))
								smMesh = *parsed;
						}
						if (partElem.contains("material") && partElem["material"].is_string())
						{
							if (auto parsed = Guid::Parse(partElem["material"].get<std::string>()))
								smMat = *parsed;
						}
						if (smMesh.IsValid() && smMat.IsValid())
						{
							meshGuids.push_back(smMesh);
							materialGuids.push_back(smMat);
							if (materialGuid.IsEmpty())
							{
								materialGuid = smMat;
							}
						}
					}
				}
			}
			else if (render.is_object())
			{
				if (render.contains("parts") && render["parts"].is_array())
				{
					for (const auto& partElem : render["parts"])
					{
						if (partElem.is_object())
						{
							Guid smMesh{};
							Guid smMat{};
							if (partElem.contains("mesh") && partElem["mesh"].is_string())
							{
								if (auto parsed = Guid::Parse(partElem["mesh"].get<std::string>()))
									smMesh = *parsed;
							}
							if (partElem.contains("material") && partElem["material"].is_string())
							{
								if (auto parsed = Guid::Parse(partElem["material"].get<std::string>()))
									smMat = *parsed;
							}
							if (smMesh.IsValid() && smMat.IsValid())
							{
								meshGuids.push_back(smMesh);
								materialGuids.push_back(smMat);
								if (materialGuid.IsEmpty())
								{
									materialGuid = smMat;
								}
							}
						}
					}
				}

				if (render.contains("mesh") && render["mesh"].is_string())
				{
					if (auto parsed = Guid::Parse(render["mesh"].get<std::string>()))
						meshGuids.push_back(*parsed);
				}

				if (render.contains("submeshes") && render["submeshes"].is_array())
				{
					for (const auto& smElem : render["submeshes"])
					{
						if (smElem.is_string())
						{
							if (auto parsed = Guid::Parse(smElem.get<std::string>()))
								meshGuids.push_back(*parsed);
						}
					}
				}

				if (render.contains("material") && render["material"].is_string())
				{
					if (auto parsed = Guid::Parse(render["material"].get<std::string>()))
						materialGuid = *parsed;
				}

				if (render.contains("materials") && render["materials"].is_array())
				{
					for (const auto& matElem : render["materials"])
					{
						if (matElem.is_string())
						{
							if (auto parsed = Guid::Parse(matElem.get<std::string>()))
								materialGuids.push_back(*parsed);
						}
					}
				}
			}
		}

		std::vector<Guid> scriptGuids;
		if (objJson.contains("scripts") && objJson["scripts"].is_array())
		{
			for (const auto& elem : objJson["scripts"])
			{
				if (elem.is_string())
				{
					if (auto parsed = Guid::Parse(elem.get<std::string>()))
						scriptGuids.push_back(*parsed);
				}
			}
		}
		else if (objJson.contains("script") && objJson["script"].is_string())
		{
			if (auto parsed = Guid::Parse(objJson["script"].get<std::string>()))
				scriptGuids.push_back(*parsed);
		}

		return GameObjectData(
			objGuid,
			std::move(name),
			isEntity,
			isActive,
			position,
			rotation,
			scale,
			std::move(meshGuids),
			materialGuid,
			std::move(scriptGuids),
			parentIndex,
			std::move(materialGuids)
		);
	}

	static void FlattenGameObjectJson(const json& objJson, uint32_t parentIndex, std::vector<GameObjectData>& outObjects)
	{
		const uint32_t myIndex = static_cast<uint32_t>(outObjects.size());
		outObjects.push_back(ParseGameObjectJson(objJson, parentIndex));

		if (objJson.contains("children") && objJson["children"].is_array())
		{
			for (const auto& childElem : objJson["children"])
			{
				if (childElem.is_object())
				{
					FlattenGameObjectJson(childElem, myIndex, outObjects);
				}
			}
		}
	}

	static SceneTransitionParams ReadTransitionParams(const json& transJson, const SceneTransitionParams& defaultParams = {})
	{
		SceneTransitionParams params = defaultParams;

		if (!transJson.is_object())
			return params;

		if (transJson.contains("type") && transJson["type"].is_string())
		{
			std::string typeStr = transJson["type"].get<std::string>();
			if (typeStr == "Instant") params.type = eTransitionType::Instant;
			else if (typeStr == "FadeColor") params.type = eTransitionType::FadeColor;
			else if (typeStr == "CrossFade") params.type = eTransitionType::CrossFade;
		}

		if (transJson.contains("duration") && transJson["duration"].is_number())
		{
			params.durationSeconds = transJson["duration"].get<zF32>();
		}
		else if (transJson.contains("durationSeconds") && transJson["durationSeconds"].is_number())
		{
			params.durationSeconds = transJson["durationSeconds"].get<zF32>();
		}

		if (transJson.contains("fadeColor"))
		{
			const auto& c = transJson["fadeColor"];
			if (c.is_string())
			{
				std::string colorName = c.get<std::string>();
				if (colorName == "CornflowerBlue") params.fadeColor = zzz::math::Palette4::CornflowerBlue;
				else if (colorName == "Black") params.fadeColor = zzz::math::Palette4::Black;
				else if (colorName == "White") params.fadeColor = zzz::math::Palette4::White;
				else if (colorName == "Red") params.fadeColor = zzz::math::Palette4::Red;
				else if (colorName == "Green") params.fadeColor = zzz::math::Palette4::Green;
				else if (colorName == "Blue") params.fadeColor = zzz::math::Palette4::Blue;
				else if (colorName == "Transparent") params.fadeColor = zzz::math::Palette4::Transparent;
			}
			else if (c.is_array() && c.size() >= 3)
			{
				zF32 r = c[0].get<zF32>();
				zF32 g = c[1].get<zF32>();
				zF32 b = c[2].get<zF32>();
				zF32 a = c.size() >= 4 ? c[3].get<zF32>() : 1.0f;
				if (r > 1.0f || g > 1.0f || b > 1.0f)
				{
					zU8 aByte = (a > 1.0f) ? static_cast<zU8>(a) : static_cast<zU8>(a * 255.0f);
					params.fadeColor = Color4<zU8>(static_cast<zU8>(r), static_cast<zU8>(g), static_cast<zU8>(b), aByte).ConvertTo<zF32>();
				}
				else
				{
					zF32 aNorm = (a > 1.0f) ? (a / 255.0f) : a;
					params.fadeColor = zzz::math::Color4<zF32>(r, g, b, aNorm);
				}
			}
		}

		if (transJson.contains("blockUserInput") && transJson["blockUserInput"].is_boolean())
			params.blockUserInput = transJson["blockUserInput"].get<bool>();

		if (transJson.contains("pauseOldSceneUpdate") && transJson["pauseOldSceneUpdate"].is_boolean())
			params.pauseOldSceneUpdate = transJson["pauseOldSceneUpdate"].get<bool>();

		if (transJson.contains("renderLoadingSpinner") && transJson["renderLoadingSpinner"].is_boolean())
			params.renderLoadingSpinner = transJson["renderLoadingSpinner"].get<bool>();

		return params;
	}

	static std::expected<std::vector<std::byte>, std::string> SerializeAssetToBinary(const PendingAsset& item, const fs::path& projectDir, zzz::core::eTargetPlatform targetPlatform, const std::string& platformConfigFile)
	{
		Serializer serializer;
		std::vector<std::byte> result;

		try
		{
			std::ifstream inFile(item.filePath);
			if (!inFile.is_open())
				return std::unexpected("Не удалось открыть файл '" + item.filePath.string() + "'.");

			json root = json::parse(inFile, nullptr, false);
			if (root.is_discarded())
			{
				return std::unexpected("Некорректный JSON в файле '" + item.filePath.string() + "'.");
			}

			auto assetType = static_cast<zzz::core::ePackage>(item.type);

			if (assetType == zzz::core::ePackage::ProjectManifest)
			{
				json configRoot = LoadPlatformConfigJson(projectDir, platformConfigFile);

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

				// Применяем deltas для game_scripts:
				// 1. remove_scripts
				if (configRoot.contains("remove_scripts") && configRoot["remove_scripts"].is_array())
				{
					std::unordered_set<Guid> removeScriptGuids;
					for (const auto& elem : configRoot["remove_scripts"])
					{
						if (elem.is_string())
						{
							if (auto parsed = Guid::Parse(elem.get<std::string>()))
								removeScriptGuids.insert(*parsed);
						}
					}
					std::erase_if(gameScriptGuids, [&](const Guid& g) {
						return removeScriptGuids.find(g) != removeScriptGuids.end();
					});
				}
				// 2. add_scripts
				if (configRoot.contains("add_scripts") && configRoot["add_scripts"].is_array())
				{
					for (const auto& elem : configRoot["add_scripts"])
					{
						if (elem.is_string())
						{
							if (auto parsed = Guid::Parse(elem.get<std::string>()))
							{
								if (std::find(gameScriptGuids.begin(), gameScriptGuids.end(), *parsed) == gameScriptGuids.end())
									gameScriptGuids.push_back(*parsed);
							}
						}
					}
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

				// Применяем deltas для scenes:
				// 1. remove_scenes
				if (configRoot.contains("remove_scenes") && configRoot["remove_scenes"].is_array())
				{
					std::unordered_set<Guid> removeSceneGuids;
					for (const auto& elem : configRoot["remove_scenes"])
					{
						if (elem.is_string())
						{
							if (auto parsed = Guid::Parse(elem.get<std::string>()))
								removeSceneGuids.insert(*parsed);
						}
					}
					std::erase_if(sceneGuids, [&](const Guid& g) {
						return removeSceneGuids.find(g) != removeSceneGuids.end();
					});
				}
				// 2. add_scenes
				if (configRoot.contains("add_scenes") && configRoot["add_scenes"].is_array())
				{
					for (const auto& elem : configRoot["add_scenes"])
					{
						if (elem.is_string())
						{
							if (auto parsed = Guid::Parse(elem.get<std::string>()))
							{
								if (std::find(sceneGuids.begin(), sceneGuids.end(), *parsed) == sceneGuids.end())
									sceneGuids.push_back(*parsed);
							}
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

				json platformRoot = ResolvePlatformJson(root, projectDir, targetPlatform, platformConfigFile);
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

				// Имя компании/приложения - единственный источник истины для каталога пользовательских
				// данных (см. Path::InitializeUserData в движке). Валидируются на стороне C# ещё до сборки
				// (см. ProjectJsonValidator.ValidateDirectoryNameField), здесь читаются как есть.
				std::string appName = root.value("app_name", root.value("name", std::string{}));
				std::string companyName = root.value("company_name", root.value("company", std::string{}));
				if (companyName.empty()) companyName = "Zzz";
				if (auto res = serializer.Serialize(result, appName); !res) return {};
				if (auto res = serializer.Serialize(result, companyName); !res) return {};

				// Версия приложения (semver-строка "major.minor.patch", например "1.0.0") - сначала
				// из платформенного конфига (build.version / build.versionName / build.bundleVersion),
				// затем из project.json (app_version / version), по умолчанию 1.0.0.
				Version appVersion{};
				std::string versionStr;
				if (configRoot.contains("build") && configRoot["build"].is_object())
				{
					const auto& b = configRoot["build"];
					if (b.contains("version") && b["version"].is_string())
						versionStr = b["version"].get<std::string>();
					else if (b.contains("versionName") && b["versionName"].is_string())
						versionStr = b["versionName"].get<std::string>();
					else if (b.contains("bundleVersion") && b["bundleVersion"].is_string())
						versionStr = b["bundleVersion"].get<std::string>();
				}
				if (versionStr.empty())
				{
					versionStr = root.value("app_version", root.value("version", "1.0.0"));
				}
				if (!versionStr.empty())
				{
					if (auto parsedVersion = Version::Parse(versionStr))
						appVersion = *parsedVersion;
				}
				if (auto res = serializer.Serialize(result, appVersion); !res) return {};

				SceneTransitionParams defaultTransitionParams{};
				if (root.contains("transition") && root["transition"].is_object())
				{
					defaultTransitionParams = ReadTransitionParams(root["transition"]);
				}
				if (auto res = serializer.Serialize(result, defaultTransitionParams); !res) return {};
			}
			else if (assetType == zzz::core::ePackage::PrimaryView)
			{
				json startViewRoot = ResolveStartViewJson(root, projectDir, targetPlatform, platformConfigFile);
				json configRoot = LoadPlatformConfigJson(projectDir, platformConfigFile);

				Guid sceneGuid{};
				if (configRoot.contains("start_scene") && configRoot["start_scene"].is_string())
				{
					if (auto parsed = Guid::Parse(configRoot["start_scene"].get<std::string>()))
						sceneGuid = *parsed;
				}
				else if (startViewRoot.contains("scene") && startViewRoot["scene"].is_string())
				{
					if (auto parsed = Guid::Parse(startViewRoot["scene"].get<std::string>()))
						sceneGuid = *parsed;
				}
				else
				{
					fs::path projJsonPath = projectDir / "project.json";
					if (fs::exists(projJsonPath))
					{
						std::ifstream projFile(projJsonPath);
						json projJson = json::parse(projFile, nullptr, false);
						if (!projJson.is_discarded() && projJson.contains("start_scene") && projJson["start_scene"].is_string())
						{
							if (auto parsed = Guid::Parse(projJson["start_scene"].get<std::string>()))
								sceneGuid = *parsed;
						}
					}
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

				Guid viewGuid = item.guid;

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

				auto clearConfig = ReadClearConfig(root);

				// Слои собираются строго из "layers". Корневой "objects" и неявный Default3DLayer удалены.
				// Каждый слой сохраняет исходный порядок и содержит обязательный валидный GUID.
				std::vector<zzz::core::LayerData> layers;

				if (root.contains("layers") && root["layers"].is_array())
				{
					for (const auto& layerElem : root["layers"])
					{
						if (!layerElem.is_object())
							continue;

						std::string layerName = layerElem.value("name", "Layer");
						Guid layerGuid{};
						if (layerElem.contains("guid") && layerElem["guid"].is_string())
						{
							if (auto parsed = Guid::Parse(layerElem["guid"].get<std::string>()))
								layerGuid = *parsed;
						}
						if (layerGuid == Guid{})
						{
							THROW_RUNTIME("Слой '{}' в сцене не содержит обязательного валидного 'guid'.", layerName);
						}
						std::string typeStr = layerElem.value("type", "Layer3D");
						eLayerType layerType = eLayerType::Layer3D;
						if (typeStr == "Layer2D" || typeStr == "LayerUI") layerType = eLayerType::Layer2D;
						else if (typeStr == "LayerMVVM") layerType = eLayerType::LayerMVVM;

						std::vector<GameObjectData> layerObjects;
						if (layerElem.contains("objects") && layerElem["objects"].is_array())
						{
							for (const auto& objElem : layerElem["objects"])
							{
								if (objElem.is_object())
									FlattenGameObjectJson(objElem, 0xFFFFFFFF, layerObjects);
							}
						}

						layers.emplace_back(layerGuid, std::move(layerName), layerType, std::move(layerObjects));
					}
				}

				eTransitionSource transitionSource = eTransitionSource::UseGlobal;
				SceneTransitionParams transitionParams{};

				if (root.contains("transitionSource") && root["transitionSource"].is_string())
				{
					std::string sourceStr = root["transitionSource"].get<std::string>();
					if (sourceStr == "Custom") transitionSource = eTransitionSource::Custom;
					else transitionSource = eTransitionSource::UseGlobal;
				}

				if (root.contains("transition") && root["transition"].is_object())
				{
					transitionParams = ReadTransitionParams(root["transition"]);
					if (!root.contains("transitionSource"))
					{
						transitionSource = eTransitionSource::Custom;
					}
				}

				zzz::core::SceneData sceneData(
					sceneScriptGuids,
					clearConfig,
					std::move(layers),
					transitionSource,
					transitionParams);

				if (auto res = serializer.Serialize(result, sceneData); !res)
					return {};
			}
			else if (assetType == zzz::core::ePackage::ChildView || assetType == zzz::core::ePackage::IndependentView)
			{
				Guid viewGuid = item.guid;

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

				if (auto res = serializer.Serialize(result, viewGuid); !res) return {};
				if (auto res = serializer.Serialize(result, sceneGuid); !res) return {};
				zU32 scriptsCount = static_cast<zU32>(uiScriptGuids.size());
				if (auto res = serializer.Serialize(result, scriptsCount); !res) return {};
				for (const auto& guid : uiScriptGuids)
				{
					if (auto res = serializer.Serialize(result, guid); !res) return {};
				}

				json platformViewRoot = ResolveStartViewJson(root, projectDir, targetPlatform, platformConfigFile);
				switch (targetPlatform)
				{
				case zzz::core::eTargetPlatform::Android:
				{
					const auto orient = ReadAndroidOrientation(platformViewRoot.value("orientation", "LandscapeLeft"));
					zU32 fps = platformViewRoot.value("targetFPS", 60u);
					const auto cutout = ReadAndroidCutoutMode(platformViewRoot.value("cutoutMode", "ShortEdges"));
					bool keepOn = platformViewRoot.value("keepScreenOn", true);

					ViewDataAndroid androidData(orient, fps, cutout, keepOn);
					if (auto res = serializer.Serialize(result, androidData); !res) return {};
					break;
				}
				case zzz::core::eTargetPlatform::Linux:
				{
					std::string title = platformViewRoot.value("title", "Game Window");
					auto size = ReadSize(platformViewRoot);
					auto windowMode = ReadLinuxWindowMode(platformViewRoot.value("windowMode", "Windowed"));
					bool resizable = platformViewRoot.value("resizable", true);
					auto displayServer = ReadLinuxDisplayServer(platformViewRoot.value("displayServer", "Auto"));

					ViewDataLinux linuxData(title, size, windowMode, resizable, displayServer);
					if (auto res = serializer.Serialize(result, linuxData); !res) return {};
					break;
				}
				case zzz::core::eTargetPlatform::MacOS:
				{
					std::string title = platformViewRoot.value("title", "Game Window");
					auto size = ReadSize(platformViewRoot);
					auto windowMode = ReadMacOSWindowMode(platformViewRoot.value("windowMode", "Windowed"));
					bool resizable = platformViewRoot.value("resizable", true);

					ViewDataMacOS macData(title, size, windowMode, resizable);
					if (auto res = serializer.Serialize(result, macData); !res) return {};
					break;
				}
				case zzz::core::eTargetPlatform::iOS:
				{
					const auto orient = ReadiOSOrientation(platformViewRoot.value("orientation", "LandscapeLeft"));
					const auto safeAreaMode = ReadiOSSafeAreaMode(platformViewRoot.value("safeAreaMode", "ExtendIntoSafeArea"));
					const auto homeIndicatorMode = ReadiOSHomeIndicatorMode(platformViewRoot.value("homeIndicatorMode", "AutoHidden"));

					ViewDataiOS iosData(orient, safeAreaMode, homeIndicatorMode);
					if (auto res = serializer.Serialize(result, iosData); !res) return {};
					break;
				}
				case zzz::core::eTargetPlatform::Windows:
				default:
				{
					std::string title = platformViewRoot.value("title", "Game Window");
					auto size = ReadSize(platformViewRoot);
					auto windowMode = ReadMSWinWindowMode(platformViewRoot);
					bool resizable = platformViewRoot.value("resizable", true);

					ViewDataMSWin winData(title, size, windowMode, resizable);
					if (auto res = serializer.Serialize(result, winData); !res) return {};
					break;
				}
				}
			}
			else
			{
				std::ifstream rawFile(item.filePath, std::ios::binary | std::ios::ate);
				if (!rawFile.is_open())
					return std::unexpected("Не удалось открыть бинарный ресурс '" + item.filePath.string() + "'.");
				const std::streampos end = rawFile.tellg();
				if (end < 0)
					return std::unexpected("Не удалось определить размер бинарного ресурса '" + item.filePath.string() + "'.");
				uint64_t size = static_cast<uint64_t>(end);
				rawFile.seekg(0, std::ios::beg);
				result.resize(size);
				if (!result.empty() && !rawFile.read(reinterpret_cast<char*>(result.data()), static_cast<std::streamsize>(size)))
					return std::unexpected("Не удалось полностью прочитать бинарный ресурс '" + item.filePath.string() + "'.");
			}
		}
		catch (const std::exception& ex)
		{
			return std::unexpected("Исключение при обработке '" + item.filePath.string() + "': " + ex.what());
		}
		catch (...)
		{
			return std::unexpected("Неизвестное исключение при обработке '" + item.filePath.string() + "'.");
		}

		return result;
	}

	bool PackagePacker::PackProject(
		const fs::path& sourceDir,
		const fs::path& destinationDir,
		zzz::core::eTargetPlatform targetPlatform,
		const std::string& platformConfigFile,
		uint64_t buildTimestamp,
		uint64_t* outBuildTimestamp)
	{
		if (outBuildTimestamp)
			*outBuildTimestamp = 0;

		char validationErrorBuf[1024]{};
		if (!ProjectIdentityValidator::Validate(sourceDir, validationErrorBuf, sizeof(validationErrorBuf), platformConfigFile))
		{
			DOutError("PackProject: Ошибка валидации идентичности проекта: {}", validationErrorBuf);
			return false;
		}

		// Единое время упаковки для package.dat и data.dat - если передано извне (из C# сборщика),
		// используется оно, чтобы buildtime-data.txt, assets_config.json и заголовки архивов
		// имели строго один и тот же штамп времени. Если 0 - генерируется здесь.
		if (buildTimestamp == 0)
		{
			buildTimestamp = static_cast<uint64_t>(
				std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch()).count());
		}

		fs::path outPath = destinationDir / zzz::core::c_GamePackageRelativePath;
		std::vector<PendingAsset> pendingAssets;

		// 1. Упаковка project.json под GUID из project.json.meta
		fs::path projJsonPath = sourceDir / "project.json";
		fs::path projMetaPath = sourceDir / "project.json.meta";
		Guid manifestGuid{};
		if (fs::exists(projMetaPath))
		{
			std::ifstream mf(projMetaPath);
			json metaJson = json::parse(mf, nullptr, false);
			if (!metaJson.is_discarded() && metaJson.contains("guid") && metaJson["guid"].is_string())
			{
				if (auto parsed = Guid::Parse(metaJson["guid"].get<std::string>()))
					manifestGuid = *parsed;
			}
		}

		if (fs::exists(projJsonPath) && manifestGuid.IsValid())
		{
			pendingAssets.push_back({
				"ProjectManifest",
				manifestGuid,
				static_cast<uint32_t>(zzz::core::ePackage::ProjectManifest),
				projJsonPath
				});
		}

		// 1.1 Списки Child/Independent вью, объявленные в платформенном конфиге (child_views/independent_views
		// внутри секции "platform" - см. ResolvePlatformJson). Это единственный
		// источник правды о том, какие вторичные окна пакуются: физическое наличие файла в Assets/ - лишь
		// необходимое условие (валидация ниже), но не достаточное. Guid не объявленный в списке не пакуется,
		// даже если ресурс физически существует на диске.
		std::unordered_set<Guid> declaredChildViewGuids;
		std::unordered_set<Guid> declaredIndependentViewGuids;
		std::unordered_set<Guid> removedSceneGuids;

		json configRoot = LoadPlatformConfigJson(sourceDir, platformConfigFile);
		if (configRoot.contains("remove_scenes") && configRoot["remove_scenes"].is_array())
		{
			for (const auto& elem : configRoot["remove_scenes"])
			{
				if (elem.is_string())
				{
					if (auto g = Guid::Parse(elem.get<std::string>()))
						removedSceneGuids.insert(*g);
				}
			}
		}

		Guid startViewGuid{};
		if (fs::exists(projJsonPath))
		{
			std::ifstream projJsonFile(projJsonPath);
			json projRoot = json::parse(projJsonFile, nullptr, false);
			if (!projRoot.is_discarded())
			{
				if (projRoot.contains("start_view") && projRoot["start_view"].is_string())
				{
					if (auto g = Guid::Parse(projRoot["start_view"].get<std::string>()))
						startViewGuid = *g;
				}

				json platformRoot = ResolvePlatformJson(projRoot, sourceDir, targetPlatform, platformConfigFile);

				if (platformRoot.contains("start_view") && platformRoot["start_view"].is_string())
				{
					if (auto g = Guid::Parse(platformRoot["start_view"].get<std::string>()))
						startViewGuid = *g;
				}

				if (platformRoot.contains("child_views") && platformRoot["child_views"].is_array())
				{
					for (const auto& elem : platformRoot["child_views"])
					{
						if (elem.is_string())
						{
							if (auto g = Guid::Parse(elem.get<std::string>()))
								declaredChildViewGuids.insert(*g);
						}
					}
				}

				if (platformRoot.contains("independent_views") && platformRoot["independent_views"].is_array())
				{
					for (const auto& elem : platformRoot["independent_views"])
					{
						if (elem.is_string())
						{
							if (auto g = Guid::Parse(elem.get<std::string>()))
								declaredIndependentViewGuids.insert(*g);
						}
					}
				}
			}
		}

		// Гуиды из деклараций выше, для которых реально нашёлся файл на диске - остальное (объявлено, но
		// не найдено) считается протухшей декларацией и логируется после скана как предупреждение.
		std::unordered_set<Guid> matchedChildViewGuids;
		std::unordered_set<Guid> matchedIndependentViewGuids;

		// 2. Поиск сцен (*.zs), вьюх (*.zv) и префабов (*.zp) в исходной директории
		// Защита от дублей имён сцен (см. также AssetsBuilderEngine.ScanProjectMetaFiles в C# -
		// там же выполняется основная, отчитывающаяся об ошибке проверка перед вызовом PackProjectNative).
		// Здесь - "тихий" защитный фильтр на случай прямого вызова нативного упаковщика в обход C#-валидации:
		// SceneManager::LoadSceneByName ищет сцену по имени в package.dat, и дубликат сделал бы поиск
		// неоднозначным (m_EntriesByName молча перезаписал бы более раннюю запись более поздней).
		std::unordered_set<std::string> seenSceneNames;

		std::vector<PendingDataAsset> pendingDataAssets;

		// Сканируем только Assets/ - project.json/build_settings/platforms на корне проекта
		// не являются ассетами и обрабатываются отдельно (см. выше). Общий AssetScanner - единая точка
		// правды обхода, используется также ProjectIdentityValidator::Validate.
		fs::path assetsScanDir = sourceDir / "Assets";
		bool assetScanOk = zzz::builder::ScanAssetsDirectory(assetsScanDir,
			[&](const zzz::builder::ScannedAssetFile& scannedFile) -> bool
			{
				const std::string& ext = scannedFile.extension;
				const fs::path& path = scannedFile.path;

				// Зарегистрированные ресурсы data.dat проходят только через единый import pipeline.
				auto importedRes = AssetImportPipeline::TryImport(path, targetPlatform);
				if (!importedRes)
				{
					DOutError("PackProject: Сборка ресурса '{}' остановлена: {}", path.string(), importedRes.error());
					return false;
				}
				if (importedRes->has_value())
				{
					auto& imported = **importedRes;
					pendingDataAssets.push_back({
						std::move(imported.name),
						imported.guid,
						imported.resourceType,
						std::move(imported.payload)
					});
					return true;
				}

				// Остальные файлы не являются входом нативного упаковщика.
				// Scene/View - структурные ресурсы package.dat, известные реестру, но не проходящие
				// через блоб-импортёр data.dat (обрабатываются структурно ниже).
				auto knownType = scannedFile.knownType;
				const bool isStructuredPackageAsset = knownType.has_value() &&
					(*knownType == zzz::core::eResourceType::Scene || *knownType == zzz::core::eResourceType::View);
				if (!isStructuredPackageAsset)
				{
					// Исходники скриптов собираются отдельно в scripts.dll, а не упаковщиком ассетов -
					// осознанное, явное исключение, а не тихий пропуск по умолчанию.
					std::string genericRel = fs::relative(path, sourceDir).generic_string();
					const bool isScriptSource = (ext == ".h" || ext == ".hpp" || ext == ".cpp") &&
						(genericRel.find("Assets/Scripts/") != std::string::npos ||
						 genericRel.find("Assets/scripts/") != std::string::npos);
					if (isScriptSource)
						return true;

					DOutError("PackProject: Незарегистрированный тип ассета '{}' для файла '{}'.", ext, path.string());
					return false;
				}

				std::string assetName = path.stem().string();
				fs::path metaPath = path.string() + ".meta";
				Guid assetGuid{};

				if (!fs::exists(metaPath) || !fs::is_regular_file(metaPath))
				{
					DOutError("PackProject: Для ресурса '{}' отсутствует обязательный мета-файл '{}'.", path.string(), metaPath.string());
					return false;
				}

				std::ifstream metaFile(metaPath);
				if (!metaFile.is_open())
				{
					DOutError("PackProject: Не удалось открыть мета-файл '{}'.", metaPath.string());
					return false;
				}
				json metaJson = json::parse(metaFile, nullptr, false);
				if (metaJson.is_discarded() || !metaJson.is_object() ||
					!metaJson.contains("guid") || !metaJson["guid"].is_string())
				{
					DOutError("PackProject: Мета-файл '{}' повреждён или не содержит строковое поле 'guid'.", metaPath.string());
					return false;
				}
				auto parsedGuid = Guid::Parse(metaJson["guid"].get<std::string>());
				if (!parsedGuid || !parsedGuid->IsValid())
				{
					DOutError("PackProject: Мета-файл '{}' содержит невалидный GUID.", metaPath.string());
					return false;
				}
				assetGuid = *parsedGuid;

				uint32_t typeVal = 0;

				if (ext == ".zs")
				{
					if (removedSceneGuids.find(assetGuid) != removedSceneGuids.end())
						return true;
					if (!seenSceneNames.insert(assetName).second)
						return true;
					typeVal = static_cast<uint32_t>(zzz::core::ePackage::Scene);
					pendingAssets.push_back({ assetName, assetGuid, typeVal, path });
				}
				else if (ext == ".zv")
				{
					// Тип вью определяется нахождением в списках JSON-конфигов (start_view, independent_views, child_views)
					bool isPrimary = (startViewGuid.IsValid() && assetGuid == startViewGuid);
					bool isIndependent = (declaredIndependentViewGuids.find(assetGuid) != declaredIndependentViewGuids.end());

					if (isPrimary)
					{
						typeVal = static_cast<uint32_t>(zzz::core::ePackage::PrimaryView);
						pendingAssets.push_back({ assetName, assetGuid, typeVal, path });
					}
					else if (isIndependent)
					{
						matchedIndependentViewGuids.insert(assetGuid);
						typeVal = static_cast<uint32_t>(zzz::core::ePackage::IndependentView);
						pendingAssets.push_back({ assetName, assetGuid, typeVal, path });
					}
					else
					{
						matchedChildViewGuids.insert(assetGuid);
						typeVal = static_cast<uint32_t>(zzz::core::ePackage::ChildView);
						pendingAssets.push_back({ assetName, assetGuid, typeVal, path });
					}
				}
				return true;
			});
		if (!assetScanOk)
			return false;

		// Диагностика: guid объявлен в child_views/independent_views платформенного конфига, но на диске
		// не найден ни одного .zv файла с таким guid - протухшая (или опечатанная) декларация.
		for (const auto& guid : declaredChildViewGuids)
			if (matchedChildViewGuids.find(guid) == matchedChildViewGuids.end())
				DOutWarning("PackProject: guid {} объявлен в child_views, но соответствующий .zv ресурс не найден в Assets/ - пропущен.", guid.ToString());

		for (const auto& guid : declaredIndependentViewGuids)
			if (matchedIndependentViewGuids.find(guid) == matchedIndependentViewGuids.end())
				DOutWarning("PackProject: guid {} объявлен в independent_views, но соответствующий .zv ресурс не найден в Assets/ - пропущен.", guid.ToString());

		bool hasPrimaryView = std::any_of(pendingAssets.begin(), pendingAssets.end(), [](const PendingAsset& item) {
			return item.type == static_cast<uint32_t>(zzz::core::ePackage::PrimaryView);
		});

		if (!hasPrimaryView)
		{
			// Если start_view не был задан явно, но есть .zv вьюхи, делаем первую из них PrimaryView
			auto firstViewIt = std::find_if(pendingAssets.begin(), pendingAssets.end(), [](const PendingAsset& item) {
				return item.type == static_cast<uint32_t>(zzz::core::ePackage::ChildView);
			});
			if (firstViewIt != pendingAssets.end())
			{
				firstViewIt->type = static_cast<uint32_t>(zzz::core::ePackage::PrimaryView);
				hasPrimaryView = true;
			}
		}

		if (!hasPrimaryView)
		{
			DOutError("PackProject: В проекте отсутствует хотя бы одно корректное представление (PrimaryView).");
			return false;
		}

		// 3. Формирование бинарного файла package.dat в подпапке destinationDir/assets/
		Serializer serializer;
		std::vector<ArchiveItem> packageItems;
		packageItems.reserve(pendingAssets.size());

		for (const auto& item : pendingAssets)
		{
			auto payloadRes = SerializeAssetToBinary(item, sourceDir, targetPlatform, platformConfigFile);
			if (!payloadRes)
			{
				DOutError("PackProject: Ошибка обработки '{}': {}", item.filePath.string(), payloadRes.error());
				return false;
			}
			if (payloadRes->empty())
			{
				DOutError("PackProject: Обработчик '{}' вернул пустой payload.", item.filePath.string());
				return false;
			}
			packageItems.push_back({
				item.name,
				item.guid,
				item.type,
				std::move(*payloadRes)
			});
		}

		// Атомарная публикация: оба архива сначала полностью пишутся во временные файлы,
		// и только при успехе обоих выполняется замена рабочих package.dat/data.dat.
		fs::path packageTmpPath = outPath;
		packageTmpPath += ".tmp";

		auto packageWriteRes = WriteBinaryArchive(
			packageTmpPath,
			c_GamePackageHeader,
			Version{ c_GamePackageFileMajorVersion, c_GamePackageFileMinorVersion, c_GamePackageFilePatchVersion },
			packageItems,
			serializer,
			buildTimestamp);
		if (!packageWriteRes)
		{
			DOutError("PackProject: Не удалось записать package.dat.tmp: {}", packageWriteRes.error());
			std::error_code cleanupEc;
			fs::remove(packageTmpPath, cleanupEc);
			return false;
		}

		// 4. Формирование бинарного архива игровых данных data.dat в destinationDir/assets/data/
		fs::path dataOutPath = destinationDir / zzz::core::c_DataPackageRelativePath;
		fs::path dataTmpPath = dataOutPath;
		dataTmpPath += ".tmp";
		std::vector<ArchiveItem> dataItems;
		dataItems.reserve(pendingDataAssets.size());

		for (auto& item : pendingDataAssets)
		{
			dataItems.push_back({
				std::move(item.name),
				item.guid,
				static_cast<uint32_t>(item.resourceType),
				std::move(item.payload)
			});
		}

		auto dataWriteRes = WriteBinaryArchive(
			dataTmpPath,
			c_DataPackageHeader,
			Version{ c_DataPackageFileMajorVersion, c_DataPackageFileMinorVersion, c_DataPackageFilePatchVersion },
			dataItems,
			serializer,
			buildTimestamp);
		if (!dataWriteRes)
		{
			DOutError("PackProject: Не удалось создать data.dat.tmp: {}", dataWriteRes.error());
			std::error_code cleanupEc;
			fs::remove(packageTmpPath, cleanupEc);
			fs::remove(dataTmpPath, cleanupEc);
			return false;
		}

		// Оба архива успешно сформированы во временных файлах - публикуем.
		// Примечание: между двумя rename нет кросс-файловой транзакции (её не даёт ни NTFS, ни POSIX
		// без отдельного журнала), поэтому крайне маловероятный сбой второго rename оставит
		// package.dat уже новым, а data.dat - ещё старым. Частично записанного/битого архива
		// в любом случае больше не возникает: до этой точки либо оба .tmp полностью готовы, либо
		// ни один рабочий файл не тронут.
		std::error_code renameEc;
		fs::rename(packageTmpPath, outPath, renameEc);
		if (renameEc)
		{
			DOutError("PackProject: Не удалось опубликовать package.dat: {}", renameEc.message());
			std::error_code cleanupEc;
			fs::remove(packageTmpPath, cleanupEc);
			fs::remove(dataTmpPath, cleanupEc);
			return false;
		}

		fs::rename(dataTmpPath, dataOutPath, renameEc);
		if (renameEc)
		{
			DOutError("PackProject: Не удалось опубликовать data.dat: {} (package.dat уже обновлён - пара архивов рассинхронизирована, требуется повторная сборка)", renameEc.message());
			std::error_code cleanupEc;
			fs::remove(dataTmpPath, cleanupEc);
			return false;
		}

		DOut("[PackagePacker] Успешно упаковано: {} (записей: {}), {} (записей: {}), buildTime: {}",
			outPath.string(), packageItems.size(), dataOutPath.string(), dataItems.size(), buildTimestamp);

		if (outBuildTimestamp)
			*outBuildTimestamp = buildTimestamp;

		return true;
	}
}
