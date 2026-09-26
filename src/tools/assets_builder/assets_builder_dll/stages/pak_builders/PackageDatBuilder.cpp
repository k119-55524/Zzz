#include "PackageDatBuilder.h"
#include <fstream>
#include <format>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include "json.hpp"
#include "core/constants/PackagesConstants.h"
#include "core/enums/ePackageDatType.h"
#include "core/enums/eLayerType.h"
#include "core/io/package/scene/SceneData.h"
#include "core/io/package/scene/LayerData.h"
#include "core/io/package/assets/GameObjectData.h"
#include "core/io/package/platforms/project/ProjectPlatformDataMSWin.h"
#include "core/io/package/platforms/project/ProjectPlatformDataLinux.h"
#include "core/io/package/platforms/project/ProjectPlatformDataAndroid.h"
#include "core/io/package/platforms/project/ProjectPlatformDataMacOS.h"
#include "core/io/package/platforms/project/ProjectPlatformDataiOS.h"
#include "core/io/package/platforms/start_view/ViewDataMSWin.h"
#include "core/io/package/platforms/start_view/ViewDataLinux.h"
#include "core/io/package/platforms/start_view/ViewDataAndroid.h"
#include "core/io/package/platforms/start_view/ViewDataMacOS.h"
#include "core/io/package/platforms/start_view/ViewDataiOS.h"
#include "core/io/package/ProjectManifestData.h"
#include "core/enums/platforms/eAndroidEnums.h"
#include "core/serialize/Serializer.h"
#include "../../ArchiveWriter.h"
#include "../../AssetExtensions.h"

using json = nlohmann::json;
using namespace zzz::core;

namespace zzz::builder
{
	namespace
	{
		std::expected<json, std::string> LoadPlatformConfigJson(const std::filesystem::path& projectDir, const std::string& platformConfigFile)
		{
			if (platformConfigFile.empty())
				return json::object();

			std::filesystem::path p(platformConfigFile);
			std::filesystem::path configPath = p.is_absolute() ? p : (projectDir / p);
			if (!std::filesystem::exists(configPath))
				configPath = projectDir / "build_settings" / p;

			if (!std::filesystem::exists(configPath))
				return UNEXPECTED("Платформенный конфигурационный файл '{}' не найден", platformConfigFile);

			std::ifstream configFile(configPath);
			if (!configFile.is_open())
				return UNEXPECTED("Не удалось открыть платформенный конфигурационный файл '{}'", configPath.string());

			json configRoot = json::parse(configFile, nullptr, false);
			if (configRoot.is_discarded())
				return UNEXPECTED("Синтаксическая ошибка JSON в платформенном конфигурационном файле '{}'", configPath.string());

			if (!configRoot.is_object())
				return UNEXPECTED("Корневой элемент платформенного конфигурационного файла '{}' должен быть JSON-объектом", configPath.string());

			return configRoot;
		}

		json ResolvePlatformJson(const json& root, const json& configRoot, eTargetPlatform)
		{
			json platformRoot = root.contains("platform") && root["platform"].is_object()
				? root["platform"]
				: json::object();

			if (configRoot.contains("platform") && configRoot["platform"].is_object())
			{
				for (const auto& [key, value] : configRoot["platform"].items())
				{
					platformRoot[key] = value;
				}
			}

			return platformRoot;
		}

		json ResolveStartViewJson(const json& root, const json& configRoot, eTargetPlatform)
		{
			json viewRoot = root;

			if (configRoot.contains("platform") && configRoot["platform"].is_object())
			{
				const auto& platform = configRoot["platform"];
				if (platform.contains("startView") && platform["startView"].is_object())
				{
					for (const auto& [key, value] : platform["startView"].items())
					{
						viewRoot[key] = value;
					}
				}
			}

			return viewRoot;
		}

		std::expected<void, std::string> SerializeProjectPlatformData(
			std::vector<std::byte>& result,
			const Serializer& serializer,
			const json& platformRoot,
			eTargetPlatform targetPlatform)
		{
			switch (targetPlatform)
			{
			case eTargetPlatform::Windows:
			{
				ProjectPlatformDataMSWin winData(platformRoot.value("windowClassName", "ZzzEngineWindowClass"));
				return serializer.Serialize(result, winData);
			}
			case eTargetPlatform::Linux:
			{
				ProjectPlatformDataLinux linuxData;
				return serializer.Serialize(result, linuxData);
			}
			case eTargetPlatform::Android:
			{
				ProjectPlatformDataAndroid androidData;
				return serializer.Serialize(result, androidData);
			}
			case eTargetPlatform::MacOS:
			{
				ProjectPlatformDataMacOS macData;
				return serializer.Serialize(result, macData);
			}
			case eTargetPlatform::iOS:
			{
				ProjectPlatformDataiOS iosData;
				return serializer.Serialize(result, iosData);
			}
			}
			return {};
		}

		Size2D<zU32> ReadSize(const json& root, zU32 defaultWidth = 1280, zU32 defaultHeight = 720)
		{
			const auto defaultSize = root.value("defaultSize", json::object());
			return Size2D<zU32>{
				root.value("width", defaultSize.value("width", defaultWidth)),
				root.value("height", defaultSize.value("height", defaultHeight))
			};
		}

		eLinuxWindowMode ReadLinuxWindowMode(std::string_view value)
		{
			if (value == "Fullscreen") return eLinuxWindowMode::Fullscreen;
			return eLinuxWindowMode::Windowed;
		}

		eLinuxDisplayServer ReadLinuxDisplayServer(std::string_view value)
		{
			if (value == "X11") return eLinuxDisplayServer::X11;
			if (value == "Wayland") return eLinuxDisplayServer::Wayland;
			return eLinuxDisplayServer::Auto;
		}

		eAndroidScreenOrientation ReadAndroidOrientation(std::string_view value)
		{
			if (value == "Sensor" || value == "AutoRotate") return eAndroidScreenOrientation::Sensor;
			if (value == "Portrait") return eAndroidScreenOrientation::Portrait;
			if (value == "LandscapeRight") return eAndroidScreenOrientation::LandscapeRight;
			return eAndroidScreenOrientation::LandscapeLeft;
		}

		eAndroidCutoutMode ReadAndroidCutoutMode(std::string_view value)
		{
			if (value == "Never") return eAndroidCutoutMode::Never;
			if (value == "Default") return eAndroidCutoutMode::Default;
			return eAndroidCutoutMode::ShortEdges;
		}

		eMacOSWindowMode ReadMacOSWindowMode(std::string_view value)
		{
			if (value == "Fullscreen") return eMacOSWindowMode::Fullscreen;
			return eMacOSWindowMode::Windowed;
		}

		eiOSScreenOrientation ReadiOSOrientation(std::string_view value)
		{
			if (value == "AutoRotate") return eiOSScreenOrientation::AutoRotate;
			if (value == "Portrait") return eiOSScreenOrientation::Portrait;
			if (value == "LandscapeRight") return eiOSScreenOrientation::LandscapeRight;
			return eiOSScreenOrientation::LandscapeLeft;
		}

		eiOSSafeAreaMode ReadiOSSafeAreaMode(std::string_view value)
		{
			if (value == "UseSafeArea") return eiOSSafeAreaMode::UseSafeArea;
			return eiOSSafeAreaMode::ExtendIntoSafeArea;
		}

		eiOSHomeIndicatorMode ReadiOSHomeIndicatorMode(std::string_view value)
		{
			if (value == "Visible") return eiOSHomeIndicatorMode::Visible;
			return eiOSHomeIndicatorMode::AutoHidden;
		}

		eMSWinWindowMode ReadMSWinWindowMode(const json& root)
		{
			const std::string windowMode = root.value("windowMode", "");
			if (windowMode == "BorderlessFullscreen") return eMSWinWindowMode::BorderlessFullscreen;
			if (windowMode == "ExclusiveFullscreen") return eMSWinWindowMode::ExclusiveFullscreen;
			if (root.value("fullscreen", false)) return eMSWinWindowMode::BorderlessFullscreen;
			return eMSWinWindowMode::Windowed;
		}

		ClearConfig ReadClearConfig(const json& root)
		{
			ClearConfig config;
			if (!root.contains("clear") || !root["clear"].is_object())
				return config;

			const auto& clearJson = root["clear"];
			std::string surfaceModeStr = clearJson.value("surfaceMode", clearJson.value("colorMode", "Color"));
			if (surfaceModeStr == "None") config.surface.mode = eSurfaceClearMode::None;
			else if (surfaceModeStr == "Shader") config.surface.mode = eSurfaceClearMode::Shader;
			else config.surface.mode = eSurfaceClearMode::Color;

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

			if (clearJson.contains("shader") && clearJson["shader"].is_string())
			{
				config.surface.shaderGuid = Guid::Parse(clearJson["shader"].get<std::string>()).value_or(Guid{});
			}

			std::string depthModeStr = clearJson.value("depthMode", "Depth");
			if (depthModeStr == "None") config.depthBuffer.depthMode = eClearDepthMode::None;
			else config.depthBuffer.depthMode = eClearDepthMode::Depth;

			config.depthBuffer.depth = clearJson.value("depth", 1.0f);

			std::string stencilModeStr = clearJson.value("stencilMode", "None");
			if (stencilModeStr == "Stencil") config.depthBuffer.stencilMode = eClearStencilMode::Stencil;
			else config.depthBuffer.stencilMode = eClearStencilMode::None;

			config.depthBuffer.stencil = clearJson.value("stencil", static_cast<zU8>(0));
			return config;
		}

		GameObjectData ParseGameObjectJson(const json& objJson, uint32_t parentIndex)
		{
			std::string name = objJson.value("name", "GameObject");
			Guid objGuid{};
			if (objJson.contains("guid") && objJson["guid"].is_string())
			{
				if (auto parsed = Guid::Parse(objJson["guid"].get<std::string>()))
					objGuid = *parsed;
			}

			bool isEntity = objJson.value("isEntity", true);
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

			std::vector<RenderPairData> renderPairs;
			if (objJson.contains("render") && objJson["render"].is_array())
			{
				for (const auto& partElem : objJson["render"])
				{
					if (partElem.is_object() && partElem.contains("mesh") && partElem.contains("material"))
					{
						auto meshParsed = Guid::Parse(partElem["mesh"].get<std::string>());
						auto matParsed = Guid::Parse(partElem["material"].get<std::string>());
						if (meshParsed && meshParsed->IsValid() && matParsed && matParsed->IsValid())
						{
							renderPairs.push_back(RenderPairData{ *meshParsed, *matParsed });
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
				std::move(renderPairs),
				std::move(scriptGuids),
				parentIndex
			);
		}

		void FlattenGameObjectJson(const json& objJson, uint32_t parentIndex, std::vector<GameObjectData>& outObjects)
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

		SceneTransitionParams ReadTransitionParams(const json& transJson, const SceneTransitionParams& defaultParams = {})
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
				params.durationSeconds = transJson["duration"].get<zF32>();
			else if (transJson.contains("durationSeconds") && transJson["durationSeconds"].is_number())
				params.durationSeconds = transJson["durationSeconds"].get<zF32>();

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
	}

	PakBuildResult PackageDatBuilder::Build(
		const StagePackPlan& plan,
		const std::filesystem::path& outputDir)
	{
		PakBuildResult result;
		result.outputFilePath = outputDir / core::c_GamePackageFileName;

		Serializer serializer;
		std::vector<ArchiveItem> packageItems;

		// 1. Сериализуем ProjectManifest
		const auto projJsonPath = plan.sourceDir / "project.json";
		std::ifstream projFile(projJsonPath);
		if (!projFile.is_open())
		{
			result.success = false;
			result.errorMessage = "Не удалось открыть project.json";
			return result;
		}

		json root = json::parse(projFile, nullptr, false);
		if (root.is_discarded() || !root.is_object())
		{
			result.success = false;
			result.errorMessage = "Ошибка парсинга project.json";
			return result;
		}

		// Читаем Guid манифеста из project.json.meta
		Guid manifestGuid{};
		auto projMetaPath = plan.sourceDir / "project.json";
		projMetaPath += c_ExtMeta;
		if (std::filesystem::exists(projMetaPath))
		{
			std::ifstream mf(projMetaPath);
			json mj = json::parse(mf, nullptr, false);
			if (!mj.is_discarded() && mj.contains("guid") && mj["guid"].is_string())
			{
				if (auto g = Guid::Parse(mj["guid"].get<std::string>()))
					manifestGuid = *g;
			}
		}

		auto configRes = LoadPlatformConfigJson(plan.sourceDir, plan.platformConfigFile);
		if (!configRes)
		{
			result.success = false;
			result.errorMessage = configRes.error();
			return result;
		}
		const json& configRoot = *configRes;
		json platformRoot = ResolvePlatformJson(root, configRoot, plan.targetPlatform);

		std::vector<std::byte> manifestPayload;
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

			std::vector<Guid> sceneGuids;
			for (const auto& scAsset : plan.activeSceneAssets)
				sceneGuids.push_back(scAsset.guid);

			std::vector<Guid> viewGuids;
			for (const auto& vAsset : plan.activeViewAssets)
				viewGuids.push_back(vAsset.guid);

			zU32 maxQueueSize = c_MaxNetworkLogQueueSize;
			zU16 loggerPort = c_DefaultLoggerPort;

			json loggerRoot = root.contains("logger") && root["logger"].is_object()
				? root["logger"]
				: (platformRoot.contains("logger") && platformRoot["logger"].is_object() ? platformRoot["logger"] : json::object());

			if (loggerRoot.contains("maxQueueSize") && loggerRoot["maxQueueSize"].is_number_integer())
				maxQueueSize = loggerRoot["maxQueueSize"].get<zU32>();
			if (loggerRoot.contains("port") && loggerRoot["port"].is_number_integer())
				loggerPort = loggerRoot["port"].get<zU16>();

			(void)serializer.Serialize(manifestPayload, static_cast<zU32>(gameScriptGuids.size()));
			for (const auto& guid : gameScriptGuids)
				(void)serializer.Serialize(manifestPayload, guid);

			(void)serializer.Serialize(manifestPayload, static_cast<zU32>(sceneGuids.size()));
			for (const auto& scAsset : plan.activeSceneAssets)
			{
				SceneManifestEntry scEntry(scAsset.relativePath.stem().string(), scAsset.guid);
				(void)serializer.Serialize(manifestPayload, scEntry);
			}

			(void)serializer.Serialize(manifestPayload, static_cast<zU32>(viewGuids.size()));
			for (const auto& guid : viewGuids)
				(void)serializer.Serialize(manifestPayload, guid);

			(void)serializer.Serialize(manifestPayload, maxQueueSize);
			(void)serializer.Serialize(manifestPayload, loggerPort);

			(void)SerializeProjectPlatformData(manifestPayload, serializer, platformRoot, plan.targetPlatform);

			std::string appName = root.value("app_name", root.value("name", std::string{}));
			std::string companyName = root.value("company_name", root.value("company", std::string{}));
			if (companyName.empty()) companyName = "Zzz";
			(void)serializer.Serialize(manifestPayload, appName);
			(void)serializer.Serialize(manifestPayload, companyName);

			Version appVersion(1, 0, 0);
			std::string versionStr = root.value("app_version", root.value("version", "1.0.0"));
			if (auto pv = Version::Parse(versionStr))
				appVersion = *pv;
			(void)serializer.Serialize(manifestPayload, appVersion);

			SceneTransitionParams defaultTransitionParams{};
			if (root.contains("transition") && root["transition"].is_object())
				defaultTransitionParams = ReadTransitionParams(root["transition"]);
			(void)serializer.Serialize(manifestPayload, defaultTransitionParams);


			packageItems.push_back({
				"ProjectManifest",
				manifestGuid,
				static_cast<uint32_t>(core::ePackageDatType::ProjectManifest),
				std::move(manifestPayload)
			});
		}

		// 2. Сериализуем Views (PrimaryView / ChildView / IndependentView)
		Guid primaryViewGuid{};
		if (platformRoot.contains("start_view") && platformRoot["start_view"].is_string())
		{
			if (auto g = Guid::Parse(platformRoot["start_view"].get<std::string>()))
				primaryViewGuid = *g;
		}
		if (primaryViewGuid.IsEmpty() && platformRoot.contains("startView") && platformRoot["startView"].is_object())
		{
			const auto& sv = platformRoot["startView"];
			if (sv.contains("view") && sv["view"].is_string())
				primaryViewGuid = Guid::Parse(sv["view"].get<std::string>()).value_or(Guid{});
			else if (sv.contains("start_view") && sv["start_view"].is_string())
				primaryViewGuid = Guid::Parse(sv["start_view"].get<std::string>()).value_or(Guid{});
		}
		if (primaryViewGuid.IsEmpty() && configRoot.contains("start_view") && configRoot["start_view"].is_string())
		{
			if (auto g = Guid::Parse(configRoot["start_view"].get<std::string>()))
				primaryViewGuid = *g;
		}
		if (primaryViewGuid.IsEmpty() && root.contains("start_view") && root["start_view"].is_string())
		{
			if (auto g = Guid::Parse(root["start_view"].get<std::string>()))
				primaryViewGuid = *g;
		}

		std::unordered_set<Guid> independentViewGuids;
		std::unordered_set<Guid> childViewGuids;

		auto collectGuids = [](const json& j, const std::string& field, std::unordered_set<Guid>& out) {
			if (j.contains(field) && j[field].is_array())
			{
				for (const auto& elem : j[field])
				{
					if (elem.is_string())
					{
						if (auto g = Guid::Parse(elem.get<std::string>()))
							out.insert(*g);
					}
				}
			}
		};

		collectGuids(root, "independent_views", independentViewGuids);
		collectGuids(root, "child_views", childViewGuids);
		collectGuids(configRoot, "independent_views", independentViewGuids);
		collectGuids(configRoot, "child_views", childViewGuids);
		if (configRoot.contains("platform") && configRoot["platform"].is_object())
		{
			collectGuids(configRoot["platform"], "independent_views", independentViewGuids);
			collectGuids(configRoot["platform"], "child_views", childViewGuids);
		}
		collectGuids(platformRoot, "independent_views", independentViewGuids);
		collectGuids(platformRoot, "child_views", childViewGuids);

		// Если primaryViewGuid не задан явно, выбираем первое окно, не помеченное явно как child или independent
		if (primaryViewGuid.IsEmpty())
		{
			for (const auto& vAsset : plan.activeViewAssets)
			{
				if (!independentViewGuids.contains(vAsset.guid) && !childViewGuids.contains(vAsset.guid))
				{
					primaryViewGuid = vAsset.guid;
					break;
				}
			}
			if (primaryViewGuid.IsEmpty() && !plan.activeViewAssets.empty())
			{
				primaryViewGuid = plan.activeViewAssets[0].guid;
			}
		}

		size_t primaryViewCount = 0;
		for (const auto& vAsset : plan.activeViewAssets)
		{
			std::ifstream vf(vAsset.fullPath);
			json viewJson = json::parse(vf, nullptr, false);
			if (viewJson.is_discarded()) continue;

			json platformViewRoot = ResolveStartViewJson(viewJson, configRoot, plan.targetPlatform);

			Guid sceneGuid{};
			if (platformViewRoot.contains("scene") && platformViewRoot["scene"].is_string())
			{
				if (auto g = Guid::Parse(platformViewRoot["scene"].get<std::string>()))
					sceneGuid = *g;
			}
			if (sceneGuid.IsEmpty() && !plan.activeSceneAssets.empty())
			{
				sceneGuid = plan.activeSceneAssets[0].guid;
			}

			std::vector<Guid> uiScriptGuids;
			if (platformViewRoot.contains("scripts") && platformViewRoot["scripts"].is_array())
			{
				for (const auto& elem : platformViewRoot["scripts"])
				{
					if (elem.is_string())
					{
						if (auto g = Guid::Parse(elem.get<std::string>()))
							uiScriptGuids.push_back(*g);
					}
				}
			}

			std::vector<std::byte> viewPayload;
			(void)serializer.Serialize(viewPayload, vAsset.guid);
			(void)serializer.Serialize(viewPayload, sceneGuid);
			(void)serializer.Serialize(viewPayload, static_cast<zU32>(uiScriptGuids.size()));
			for (const auto& g : uiScriptGuids)
				(void)serializer.Serialize(viewPayload, g);

			// Платформенные параметры окна
			switch (plan.targetPlatform)
			{
			case eTargetPlatform::Linux:
			{
				std::string title = platformViewRoot.value("title", "Game Window");
				auto size = ReadSize(platformViewRoot);
				auto windowMode = ReadLinuxWindowMode(platformViewRoot.value("windowMode", "Windowed"));
				bool resizable = platformViewRoot.value("resizable", true);
				auto displayServer = ReadLinuxDisplayServer(platformViewRoot.value("displayServer", "Auto"));

				ViewDataLinux linuxData(title, size, windowMode, resizable, displayServer);
				(void)serializer.Serialize(viewPayload, linuxData);
				break;
			}
			case eTargetPlatform::Android:
			{
				auto orientation = ReadAndroidOrientation(platformViewRoot.value("orientation", "LandscapeLeft"));
				zU32 targetFps = platformViewRoot.value("targetFPS", 60);
				auto cutoutMode = ReadAndroidCutoutMode(platformViewRoot.value("cutoutMode", "ShortEdges"));
				bool keepScreenOn = platformViewRoot.value("keepScreenOn", true);

				ViewDataAndroid androidData(orientation, targetFps, cutoutMode, keepScreenOn);
				(void)serializer.Serialize(viewPayload, androidData);
				break;
			}
			case eTargetPlatform::MacOS:
			{
				std::string title = platformViewRoot.value("title", "Game Window");
				auto size = ReadSize(platformViewRoot);
				auto windowMode = ReadMacOSWindowMode(platformViewRoot.value("windowMode", "Windowed"));
				bool resizable = platformViewRoot.value("resizable", true);

				ViewDataMacOS macData(title, size, windowMode, resizable);
				(void)serializer.Serialize(viewPayload, macData);
				break;
			}
			case eTargetPlatform::iOS:
			{
				const auto orientation = ReadiOSOrientation(platformViewRoot.value("orientation", "LandscapeLeft"));
				const auto safeAreaMode = ReadiOSSafeAreaMode(platformViewRoot.value("safeAreaMode", "ExtendIntoSafeArea"));
				const auto homeIndicatorMode = ReadiOSHomeIndicatorMode(platformViewRoot.value("homeIndicatorMode", "AutoHidden"));

				ViewDataiOS iosData(orientation, safeAreaMode, homeIndicatorMode);
				(void)serializer.Serialize(viewPayload, iosData);
				break;
			}

			case eTargetPlatform::Windows:
			default:
			{
				std::string title = platformViewRoot.value("title", "Game Window");
				auto size = ReadSize(platformViewRoot);
				auto windowMode = ReadMSWinWindowMode(platformViewRoot);
				bool resizable = platformViewRoot.value("resizable", true);

				ViewDataMSWin winData(title, size, windowMode, resizable);
				(void)serializer.Serialize(viewPayload, winData);
				break;
			}
			}

			auto checkTypeStr = [](const json& j, const std::string& field) -> std::optional<core::ePackageDatType> {
				if (j.contains(field) && j[field].is_string())
				{
					std::string val = j[field].get<std::string>();
					if (val == "PrimaryView" || val == "Primary") return core::ePackageDatType::PrimaryView;
					if (val == "IndependentView" || val == "Independent") return core::ePackageDatType::IndependentView;
					if (val == "ChildView" || val == "Child") return core::ePackageDatType::ChildView;
				}
				return std::nullopt;
			};

			std::optional<core::ePackageDatType> explicitRole;
			if (auto r = checkTypeStr(platformViewRoot, "role")) explicitRole = r;
			else if (auto r2 = checkTypeStr(platformViewRoot, "type")) explicitRole = r2;
			else if (auto r3 = checkTypeStr(platformViewRoot, "viewType")) explicitRole = r3;
			else if (auto r4 = checkTypeStr(viewJson, "role")) explicitRole = r4;
			else if (auto r5 = checkTypeStr(viewJson, "type")) explicitRole = r5;
			else if (auto r6 = checkTypeStr(viewJson, "viewType")) explicitRole = r6;

			core::ePackageDatType viewType;
			if (vAsset.guid == primaryViewGuid)
			{
				viewType = core::ePackageDatType::PrimaryView;
			}
			else if (independentViewGuids.contains(vAsset.guid) || explicitRole == core::ePackageDatType::IndependentView)
			{
				viewType = core::ePackageDatType::IndependentView;
			}
			else if (childViewGuids.contains(vAsset.guid) || explicitRole == core::ePackageDatType::ChildView)
			{
				viewType = core::ePackageDatType::ChildView;
			}
			else if (explicitRole.has_value())
			{
				viewType = *explicitRole;
			}
			else if (primaryViewCount == 0)
			{
				viewType = core::ePackageDatType::PrimaryView;
			}
			else
			{
				viewType = core::ePackageDatType::ChildView;
			}

			if (viewType == core::ePackageDatType::PrimaryView)
			{
				primaryViewCount++;
			}

			packageItems.push_back({
				vAsset.relativePath.stem().string(),
				vAsset.guid,
				static_cast<uint32_t>(viewType),
				std::move(viewPayload)
			});
		}

		if (!plan.activeViewAssets.empty())
		{
			if (primaryViewCount == 0)
			{
				result.success = false;
				result.errorMessage = "Не найдено ни одного PrimaryView для упаковки в package.dat";
				return result;
			}
			if (primaryViewCount > 1)
			{
				result.success = false;
				result.errorMessage = std::format("Обнаружено {} PrimaryView для упаковки в package.dat. Допустим ровно один PrimaryView.", primaryViewCount);
				return result;
			}
		}

		// 3. Сериализуем Scenes
		for (const auto& scAsset : plan.activeSceneAssets)
		{
			std::ifstream sf(scAsset.fullPath);
			json sceneJson = json::parse(sf, nullptr, false);
			if (sceneJson.is_discarded()) continue;

			std::vector<Guid> sceneScriptGuids;
			if (sceneJson.contains("scripts") && sceneJson["scripts"].is_array())
			{
				for (const auto& elem : sceneJson["scripts"])
				{
					if (elem.is_string())
					{
						if (auto g = Guid::Parse(elem.get<std::string>()))
							sceneScriptGuids.push_back(*g);
					}
				}
			}
			else if (sceneJson.contains("script") && sceneJson["script"].is_string())
			{
				if (auto g = Guid::Parse(sceneJson["script"].get<std::string>()))
					sceneScriptGuids.push_back(*g);
			}

			auto clearConfig = ReadClearConfig(sceneJson);

			std::vector<core::LayerData> layers;
			if (sceneJson.contains("layers") && sceneJson["layers"].is_array())
			{
				for (const auto& layerElem : sceneJson["layers"])
				{
					if (!layerElem.is_object()) continue;

					std::string layerName = layerElem.value("name", "Layer");
					Guid layerGuid{};
					if (layerElem.contains("guid") && layerElem["guid"].is_string())
					{
						if (auto g = Guid::Parse(layerElem["guid"].get<std::string>()))
							layerGuid = *g;
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
			if (sceneJson.contains("transitionSource") && sceneJson["transitionSource"].is_string())
			{
				if (sceneJson["transitionSource"].get<std::string>() == "Custom")
					transitionSource = eTransitionSource::Custom;
			}
			if (sceneJson.contains("transition") && sceneJson["transition"].is_object())
			{
				transitionParams = ReadTransitionParams(sceneJson["transition"]);
				if (!sceneJson.contains("transitionSource"))
					transitionSource = eTransitionSource::Custom;
			}

			core::SceneData sceneData(
				sceneScriptGuids,
				clearConfig,
				std::move(layers),
				transitionSource,
				transitionParams);

			std::vector<std::byte> scenePayload;
			(void)serializer.Serialize(scenePayload, sceneData);

			packageItems.push_back({
				scAsset.relativePath.stem().string(),
				scAsset.guid,
				static_cast<uint32_t>(core::ePackageDatType::Scene),
				std::move(scenePayload)
			});
		}

		// 4. Записываем package.dat через WriteBinaryArchive
		auto writeRes = WriteBinaryArchive(
			result.outputFilePath,
			core::c_PackageDatFormat,
			packageItems,
			serializer,
			plan.buildTimestamp);

		if (!writeRes)
		{
			result.success = false;
			result.errorMessage = std::format("Ошибка записи package.dat: {}", writeRes.error());
			return result;
		}

		result.success = true;
		result.outputFileSize = std::filesystem::file_size(result.outputFilePath);
		return result;
	}
}
