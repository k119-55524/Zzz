#include "Stage2_PackPlanner.h"
#include <fstream>
#include <format>
#include <unordered_set>
#include <queue>
#include "json.hpp"
#include "core/logger/logger.h"
#include "core/constants/LogCategoryConstants.h"
#include "TextureBuilder.h"
#include "../AssetImporterRegistry.h"

namespace zzz::builder
{
	namespace
	{
		std::expected<nlohmann::json, std::string> LoadPlatformConfigJson(const std::filesystem::path& projectDir, const std::string& platformConfigFile)
		{
			if (platformConfigFile.empty())
				return nlohmann::json::object();

			std::filesystem::path p(platformConfigFile);
			std::filesystem::path configPath = p.is_absolute() ? p : (projectDir / p);
			if (!std::filesystem::exists(configPath))
			{
				configPath = projectDir / "build_settings" / p;
			}

			if (!std::filesystem::exists(configPath))
			{
				return UNEXPECTED("Платформенный конфигурационный файл '{}' не найден", platformConfigFile);
			}

			std::ifstream configFile(configPath);
			if (!configFile.is_open())
			{
				return UNEXPECTED("Не удалось открыть платформенный конфигурационный файл '{}'", configPath.string());
			}

			nlohmann::json configRoot = nlohmann::json::parse(configFile, nullptr, false);
			if (configRoot.is_discarded())
			{
				return UNEXPECTED("Синтаксическая ошибка JSON в платформенном конфигурационном файле '{}'", configPath.string());
			}

			if (!configRoot.is_object())
			{
				return UNEXPECTED("Корневой элемент платформенного конфигурационного файла '{}' должен быть JSON-объектом", configPath.string());
			}

			return configRoot;
		}

		void ExtractObjectDependencies(
			const nlohmann::json& objElem,
			std::vector<core::Guid>& outMeshes,
			std::vector<core::Guid>& outMaterials,
			std::vector<core::Guid>& outScripts,
			std::vector<core::Guid>& outAudio)
		{
			if (!objElem.is_object())
				return;

			if (objElem.contains("script") && objElem["script"].is_string())
			{
				if (auto g = core::Guid::Parse(objElem["script"].get<std::string>()))
					outScripts.push_back(*g);
			}
			if (objElem.contains("scripts") && objElem["scripts"].is_array())
			{
				for (const auto& elem : objElem["scripts"])
				{
					if (elem.is_string())
					{
						if (auto g = core::Guid::Parse(elem.get<std::string>()))
							outScripts.push_back(*g);
					}
				}
			}

			if (objElem.contains("audio") && objElem["audio"].is_string())
			{
				if (auto g = core::Guid::Parse(objElem["audio"].get<std::string>()))
					outAudio.push_back(*g);
			}

			if (objElem.contains("render") && objElem["render"].is_array())
			{
				for (const auto& partElem : objElem["render"])
				{
					if (partElem.is_object())
					{
						if (partElem.contains("mesh") && partElem["mesh"].is_string())
						{
							if (auto g = core::Guid::Parse(partElem["mesh"].get<std::string>()))
								outMeshes.push_back(*g);
						}
						if (partElem.contains("material") && partElem["material"].is_string())
						{
							if (auto g = core::Guid::Parse(partElem["material"].get<std::string>()))
								outMaterials.push_back(*g);
						}
					}
				}
			}

			if (objElem.contains("children") && objElem["children"].is_array())
			{
				for (const auto& child : objElem["children"])
				{
					ExtractObjectDependencies(child, outMeshes, outMaterials, outScripts, outAudio);
				}
			}
		}
	}

	StagePackPlan Stage2_PackPlanner::Plan(
		const StageValidationResult& valResult,
		const std::filesystem::path& sourceDir,
		const std::filesystem::path& destinationDir,
		core::eTargetPlatform targetPlatform,
		const std::string& platformConfigFile,
		uint64_t buildTimestamp)
	{
		StagePackPlan plan;
		plan.sourceDir = sourceDir;
		plan.destinationDir = destinationDir;
		plan.targetPlatform = targetPlatform;
		plan.platformConfigFile = platformConfigFile;
		plan.buildTimestamp = buildTimestamp;

		// 1. Проверяем валидность исходных результатов Стадии 1
		if (!valResult.isValid)
		{
			plan.isValid = false;
			plan.errorMessage = "Невозможно построить план упаковки: проект не прошел Стадию 1 (" + valResult.errorMessage + ")";
			return plan;
		}

		// 2. Читаем project.json
		const auto projJsonPath = sourceDir / "project.json";
		if (!std::filesystem::exists(projJsonPath))
		{
			plan.isValid = false;
			plan.errorMessage = "В корне проекта отсутствует 'project.json'";
			return plan;
		}

		nlohmann::json projJson;
		{
			std::ifstream f(projJsonPath);
			projJson = nlohmann::json::parse(f, nullptr, false);
			if (projJson.is_discarded() || !projJson.is_object())
			{
				plan.isValid = false;
				plan.errorMessage = "Ошибка парсинга 'project.json'";
				return plan;
			}
		}

		// 3. Платформенный конфиг
		auto configRes = LoadPlatformConfigJson(sourceDir, platformConfigFile);
		if (!configRes)
		{
			plan.isValid = false;
			plan.errorMessage = configRes.error();
			return plan;
		}
		const nlohmann::json& configRoot = *configRes;

		std::unordered_set<core::Guid> removedSceneGuids;
		if (configRoot.contains("remove_scenes") && configRoot["remove_scenes"].is_array())
		{
			for (const auto& elem : configRoot["remove_scenes"])
			{
				if (elem.is_string())
				{
					if (auto g = core::Guid::Parse(elem.get<std::string>()))
						removedSceneGuids.insert(*g);
				}
			}
		}

		std::unordered_set<core::Guid> addedSceneGuids;
		if (configRoot.contains("add_scenes") && configRoot["add_scenes"].is_array())
		{
			for (const auto& elem : configRoot["add_scenes"])
			{
				if (elem.is_string())
				{
					if (auto g = core::Guid::Parse(elem.get<std::string>()))
						addedSceneGuids.insert(*g);
				}
			}
		}

		// 4. Определение активных сцен и вьюх
		std::unordered_set<core::Guid> activeSceneGuids;
		std::unordered_set<core::Guid> activeViewGuids;

		// Базовые сцены из project.json
		if (projJson.contains("start_scene") && projJson["start_scene"].is_string())
		{
			if (auto g = core::Guid::Parse(projJson["start_scene"].get<std::string>()))
				activeSceneGuids.insert(*g);
		}
		if (projJson.contains("scenes") && projJson["scenes"].is_array())
		{
			for (const auto& elem : projJson["scenes"])
			{
				if (elem.is_string())
				{
					if (auto g = core::Guid::Parse(elem.get<std::string>()))
						activeSceneGuids.insert(*g);
				}
			}
		}

		// Сцены по умолчанию (если в project.json список пуст, берём все обнаруженные)
		if (activeSceneGuids.empty())
		{
			for (const auto& asset : valResult.allAssets)
			{
				if (asset.resourceType == core::eEngineResourceType::Scene)
					activeSceneGuids.insert(asset.guid);
			}
		}

		// Вьюхи из project.json и platform config
		if (projJson.contains("start_view") && projJson["start_view"].is_string())
		{
			if (auto g = core::Guid::Parse(projJson["start_view"].get<std::string>()))
				activeViewGuids.insert(*g);
		}
		if (projJson.contains("views") && projJson["views"].is_array())
		{
			for (const auto& elem : projJson["views"])
			{
				if (elem.is_string())
				{
					if (auto g = core::Guid::Parse(elem.get<std::string>()))
						activeViewGuids.insert(*g);
				}
			}
		}
		if (configRoot.contains("platform") && configRoot["platform"].is_object())
		{
			const auto& plat = configRoot["platform"];
			if (plat.contains("start_view") && plat["start_view"].is_string())
			{
				if (auto g = core::Guid::Parse(plat["start_view"].get<std::string>()))
					activeViewGuids.insert(*g);
			}
			if (plat.contains("child_views") && plat["child_views"].is_array())
			{
				for (const auto& elem : plat["child_views"])
				{
					if (elem.is_string())
					{
						if (auto g = core::Guid::Parse(elem.get<std::string>()))
							activeViewGuids.insert(*g);
					}
				}
			}
			if (plat.contains("independent_views") && plat["independent_views"].is_array())
			{
				for (const auto& elem : plat["independent_views"])
				{
					if (elem.is_string())
					{
						if (auto g = core::Guid::Parse(elem.get<std::string>()))
							activeViewGuids.insert(*g);
					}
				}
			}
		}

		if (activeViewGuids.empty())
		{
			for (const auto& asset : valResult.allAssets)
			{
				if (asset.resourceType == core::eEngineResourceType::View)
					activeViewGuids.insert(asset.guid);
			}
		}

		// 5. Раскрутка графа зависимостей (BFS / DFS с visited-set)
		std::unordered_set<core::Guid> reachableGuids;
		std::queue<core::Guid> toProcess;

		// Сначала из активных View извлекаем сцены
		for (const auto& viewGuid : activeViewGuids)
		{
			const auto* asset = valResult.FindAsset(viewGuid);
			if (!asset) continue;

			plan.activeViewAssets.push_back(*asset);
			reachableGuids.insert(viewGuid);

			std::ifstream vf(asset->fullPath);
			if (vf.is_open())
			{
				nlohmann::json viewJson = nlohmann::json::parse(vf, nullptr, false);
				if (!viewJson.is_discarded() && viewJson.is_object())
				{
					if (viewJson.contains("scene") && viewJson["scene"].is_string())
					{
						if (auto g = core::Guid::Parse(viewJson["scene"].get<std::string>()))
						{
							if (!removedSceneGuids.contains(*g))
							{
								activeSceneGuids.insert(*g);
							}
						}
					}
				}
			}
		}

		// Применяем add_scenes и remove_scenes (после извлечения сцен из views)
		for (const auto& g : addedSceneGuids)
			activeSceneGuids.insert(g);
		for (const auto& g : removedSceneGuids)
			activeSceneGuids.erase(g);

		// Добавляем активные сцены в очередь
		for (const auto& sceneGuid : activeSceneGuids)
		{
			const auto* asset = valResult.FindAsset(sceneGuid);
			if (!asset)
			{
				plan.isValid = false;
				plan.errorMessage = std::format("Активная сцена с GUID '{}' не найдена среди ресурсов проекта", sceneGuid.ToString());
				return plan;
			}

			plan.activeSceneAssets.push_back(*asset);
			plan.activeSceneNames.push_back(asset->relativePath.stem().string());
			reachableGuids.insert(sceneGuid);
			toProcess.push(sceneGuid);
		}

		// Обход графа
		while (!toProcess.empty())
		{
			const auto curGuid = toProcess.front();
			toProcess.pop();

			const auto* asset = valResult.FindAsset(curGuid);
			if (!asset)
				continue;

			if (asset->resourceType == core::eEngineResourceType::Scene)
			{
				std::ifstream sf(asset->fullPath);
				if (!sf.is_open()) continue;
				nlohmann::json sj = nlohmann::json::parse(sf, nullptr, false);
				if (sj.is_discarded() || !sj.is_object()) continue;

				std::vector<core::Guid> meshes;
				std::vector<core::Guid> materials;
				std::vector<core::Guid> scripts;
				std::vector<core::Guid> audio;

				if (sj.contains("layers") && sj["layers"].is_array())
				{
					for (const auto& layer : sj["layers"])
					{
						if (layer.is_object() && layer.contains("objects") && layer["objects"].is_array())
						{
							for (const auto& obj : layer["objects"])
							{
								ExtractObjectDependencies(obj, meshes, materials, scripts, audio);
							}
						}
					}
				}

				for (const auto& g : meshes)
				{
					if (reachableGuids.insert(g).second)
						toProcess.push(g);
				}
				for (const auto& g : materials)
				{
					if (reachableGuids.insert(g).second)
						toProcess.push(g);
				}
				for (const auto& g : scripts)
				{
					reachableGuids.insert(g);
				}
				for (const auto& g : audio)
				{
					if (reachableGuids.insert(g).second)
						toProcess.push(g);
				}
			}
			else if (asset->resourceType == core::eEngineResourceType::Material)
			{
				std::ifstream mf(asset->fullPath);
				if (!mf.is_open()) continue;
				nlohmann::json mj = nlohmann::json::parse(mf, nullptr, false);
				if (mj.is_discarded() || !mj.is_object()) continue;

				if (mj.contains("shader") && mj["shader"].is_string())
				{
					if (auto g = core::Guid::Parse(mj["shader"].get<std::string>()))
					{
						if (reachableGuids.insert(*g).second)
							toProcess.push(*g);
					}
				}

				if (mj.contains("textures") && mj["textures"].is_array())
				{
					for (const auto& elem : mj["textures"])
					{
						if (elem.is_string())
						{
							if (auto g = core::Guid::Parse(elem.get<std::string>()))
							{
								if (reachableGuids.insert(*g).second)
									toProcess.push(*g);
							}
						}
					}
				}
				if (mj.contains("texture") && mj["texture"].is_string())
				{
					if (auto g = core::Guid::Parse(mj["texture"].get<std::string>()))
					{
						if (reachableGuids.insert(*g).second)
							toProcess.push(*g);
					}
				}
			}
		}

		// 6. Классификация и Dead-Code Stripping
		for (const auto& asset : valResult.allAssets)
		{
			if (asset.resourceType == core::eEngineResourceType::Scene ||
				asset.resourceType == core::eEngineResourceType::View)
			{
				continue; // Обработаны отдельно
			}

			// Data-ресурсы: если достижимы — раскладываем по корзинам, иначе отсекаем

			if (asset.dataDatType.has_value())
			{
				if (reachableGuids.contains(asset.guid))
				{
					const auto ext = asset.fullPath.extension().string();
					auto importer = AssetImporterRegistry::Instance().GetImporter(ext);
					if (!importer)
					{
						plan.isValid = false;
						plan.errorMessage = std::format(
							"Достижимый ресурс '{}' [{}] типа '{}' не имеет зарегистрированного импортёра",
							asset.relativePath.string(), asset.guid.ToString(), core::ToString(*asset.dataDatType));
						DOutError("{}", plan.errorMessage);
						return plan;
					}

					plan.assetsByPak[*asset.dataDatType].push_back(asset);
				}
				else
				{
					plan.strippedAssets.push_back(asset);
				}
			}
		}

		// 7. Probe текстур через TextureBuilder
		zzz::texture::TextureBuilder texBuilder;
		if (auto it = plan.assetsByPak.find(core::eDataDatType::Texture2D); it != plan.assetsByPak.end())
		{
			for (const auto& texAsset : it->second)
			{
				ProbedTextureInfo pInfo;
				pInfo.guid = texAsset.guid;
				pInfo.relativePath = texAsset.relativePath.string();

				std::ifstream f(texAsset.fullPath, std::ios::binary);
				if (f.is_open())
				{
					std::vector<zU8> bytes(std::istreambuf_iterator<char>(f), {});
					auto probeRes = texBuilder.Probe(bytes);
					if (probeRes)
					{
						pInfo.success = true;
						pInfo.metadata.width = probeRes->width;
						pInfo.metadata.height = probeRes->height;
						pInfo.sourceFormatName = std::format("{}x{}, {}ch", probeRes->width, probeRes->height, probeRes->channels);
					}
					else
					{
						pInfo.success = false;
						pInfo.errorMessage = probeRes.error();
					}
				}
				plan.probedTextures[texAsset.guid] = std::move(pInfo);
			}
		}

		// 8. Логирование подробной сводной таблицы состава ресурсов
		DOut(Assets, "================================================================================");
		DOut(Assets, " ПЛАН СБОРКИ РЕСУРСОВ ТАРГЕТА: {}", core::ToString(targetPlatform));
		DOut(Assets, "================================================================================");
		DOut(Assets, " Активные сцены ({}):", plan.activeSceneNames.size());
		for (const auto& name : plan.activeSceneNames)
		{
			DOut(Assets, "   • {}", name);
		}

		DOut(Assets, " Встроенные ресурсы (data.dat inline):");
		auto logTypeCount = [&](core::eDataDatType type, std::string_view label) {
			const auto it = plan.assetsByPak.find(type);
			const std::size_t count = (it != plan.assetsByPak.end()) ? it->second.size() : 0;
			DOut(Assets, "   • {}: {}", label, count);
		};
		logTypeCount(core::eDataDatType::Mesh, "Mesh (сетки)");
		logTypeCount(core::eDataDatType::Material, "Material (материалы)");
		logTypeCount(core::eDataDatType::Shader, "Shader (шейдеры)");
		logTypeCount(core::eDataDatType::Animation, "Animation (анимации)");

		DOut(Assets, " Внешние пакеты (.dat):");
		if (auto it = plan.assetsByPak.find(core::eDataDatType::Texture2D); it != plan.assetsByPak.end())
		{
			DOut(Assets, "   • {}: {} текстур", core::GetPakFileName(core::eDataDatType::Texture2D), it->second.size());
			for (const auto& tex : it->second)
			{
				const auto& p = plan.probedTextures[tex.guid];
				DOut(Assets, "       - {} [{}] ({})", tex.relativePath.string(), tex.guid.ToString(), p.sourceFormatName);
			}
		}
		if (auto it = plan.assetsByPak.find(core::eDataDatType::AudioClip); it != plan.assetsByPak.end())
		{
			DOut(Assets, "   • {}: {} аудиофайлов", core::GetPakFileName(core::eDataDatType::AudioClip), it->second.size());

			for (const auto& snd : it->second)
			{
				DOut(Assets, "       - {} [{}]", snd.relativePath.string(), snd.guid.ToString());
			}
		}

		DOut(Assets, " Отсеянные ресурсы (Dead-code stripped: {}):", plan.strippedAssets.size());
		for (const auto& strp : plan.strippedAssets)
		{
			DOut(Assets, "   ✕ {} [{}]", strp.relativePath.string(), strp.guid.ToString());
		}
		DOut(Assets, "================================================================================");

		plan.isValid = true;
		return plan;
	}
}
