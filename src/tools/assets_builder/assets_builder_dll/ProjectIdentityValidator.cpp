#include "ProjectIdentityValidator.h"
#include "AssetExtensions.h"
#include "core/utils/Guid.h"
#include "json.hpp"

#include <algorithm>
#include <cstring>
#include <format>
#include <fstream>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;
using json = nlohmann::json;
using namespace zzz::core;
using namespace zzz::builder;

namespace zzz::builder
{
	std::string_view ToString(GuidOwnerKind kind) noexcept
	{
		switch (kind)
		{
		case GuidOwnerKind::Project:   return "Project";
		case GuidOwnerKind::Scene:     return "Scene";
		case GuidOwnerKind::View:      return "View";
		case GuidOwnerKind::Script:    return "Script";
		case GuidOwnerKind::Mesh:      return "Mesh";
		case GuidOwnerKind::Material:  return "Material";
		case GuidOwnerKind::Shader:    return "Shader";
		case GuidOwnerKind::Texture:   return "Texture";
		case GuidOwnerKind::Prefab:    return "Prefab";
		case GuidOwnerKind::Layer:     return "Layer";
		case GuidOwnerKind::Object:    return "Object";
		}
		return "Unknown";
	}

	namespace
	{
		struct GuidOwnerInfo
		{
			Guid guid;
			GuidOwnerKind kind;
			fs::path sourcePath;
			std::string jsonPath;
		};

		void SetError(char* outBuffer, uint32_t bufferSize, std::string_view msg) noexcept
		{
			if (!outBuffer || bufferSize == 0)
				return;
			const size_t copyLen = std::min<size_t>(msg.size(), bufferSize - 1);
			std::memcpy(outBuffer, msg.data(), copyLen);
			if (copyLen < msg.size() && bufferSize > 4)
			{
				outBuffer[bufferSize - 4] = '.';
				outBuffer[bufferSize - 3] = '.';
				outBuffer[bufferSize - 2] = '.';
			}
			outBuffer[copyLen] = '\0';
		}

		bool RegisterGuid(
			std::unordered_map<Guid, GuidOwnerInfo>& registry,
			const Guid& guid,
			GuidOwnerKind kind,
			const fs::path& sourcePath,
			std::string_view jsonPath,
			std::string& outError)
		{
			if (guid.IsEmpty())
			{
				outError = std::format("Файл '{}'{}: обнаружен пустой или нулевой GUID владельца.",
					sourcePath.string(),
					jsonPath.empty() ? "" : std::format(" ({})", jsonPath));
				return false;
			}

			auto it = registry.find(guid);
			if (it != registry.end())
			{
				const auto& existing = it->second;
				outError = std::format(
					"Обнаружен дубликат GUID '{}' между {} ('{}'{}) и {} ('{}'{}).",
					guid.ToString(),
					ToString(existing.kind), existing.sourcePath.string(),
					existing.jsonPath.empty() ? "" : std::format(" {}", existing.jsonPath),
					ToString(kind), sourcePath.string(),
					jsonPath.empty() ? "" : std::format(" {}", jsonPath));
				return false;
			}

			registry[guid] = GuidOwnerInfo{
				.guid = guid,
				.kind = kind,
				.sourcePath = sourcePath,
				.jsonPath = std::string(jsonPath)
			};
			return true;
		}

		bool RegisterObjectsRecursively(
			std::unordered_map<Guid, GuidOwnerInfo>& registry,
			const json& objectsArray,
			const fs::path& sceneRelPath,
			const std::string& parentJsonPath,
			std::string& outError)
		{
			if (!objectsArray.is_array())
			{
				outError = std::format("Сцена '{}': элемент '{}' должен быть массивом JSON.",
					sceneRelPath.string(), parentJsonPath);
				return false;
			}

			for (size_t i = 0; i < objectsArray.size(); ++i)
			{
				const auto& objElem = objectsArray[i];
				std::string currentPath = std::format("{}[{}]", parentJsonPath, i);

				if (!objElem.is_object())
				{
					outError = std::format("Сцена '{}': элемент '{}' не является объектом JSON.",
						sceneRelPath.string(), currentPath);
					return false;
				}

				if (objElem.contains("name") && !objElem["name"].is_string())
				{
					outError = std::format("Сцена '{}', объект {}: поле 'name' должно быть строкой.",
						sceneRelPath.string(), currentPath);
					return false;
				}

				std::string objName = objElem.value("name", "Object");

				if (!objElem.contains("guid") || !objElem["guid"].is_string())
				{
					outError = std::format("Сцена '{}', объект {} ('{}'): отсутствует обязательное строковое поле 'guid'.",
						sceneRelPath.string(), currentPath, objName);
					return false;
				}

				auto parsed = Guid::Parse(objElem["guid"].get<std::string>());
				if (!parsed)
				{
					outError = std::format("Сцена '{}', объект {} ('{}'): некорректный формат GUID '{}'.",
						sceneRelPath.string(), currentPath, objName, objElem["guid"].get<std::string>());
					return false;
				}

				if (!RegisterGuid(registry, *parsed, GuidOwnerKind::Object, sceneRelPath, currentPath, outError))
					return false;

				if (objElem.contains("children"))
				{
					if (!objElem["children"].is_array())
					{
						outError = std::format("Сцена '{}', объект {} ('{}'): поле 'children' должно быть массивом.",
							sceneRelPath.string(), currentPath, objName);
						return false;
					}

					if (!RegisterObjectsRecursively(registry, objElem["children"], sceneRelPath, currentPath + ".children", outError))
						return false;
				}
			}

			return true;
		}

		bool ValidateGuidRef(
			const std::unordered_map<Guid, GuidOwnerInfo>& registry,
			const std::string& guidStr,
			GuidOwnerKind expectedKind,
			std::string_view context,
			std::string& outError,
			bool allowEmpty = false)
		{
			if (guidStr.empty())
			{
				if (allowEmpty)
					return true;

				outError = std::format("{}: пустая строка не является допустимым GUID.", context);
				return false;
			}

			auto parsed = Guid::Parse(guidStr);
			if (!parsed)
			{
				outError = std::format("{}: ссылка '{}' не является валидным GUID.", context, guidStr);
				return false;
			}

			auto it = registry.find(*parsed);
			if (it == registry.end())
			{
				outError = std::format("{}: ссылка на GUID '{}' указывает на несуществующий ресурс/владелец.",
					context, guidStr);
				return false;
			}

			if (it->second.kind != expectedKind)
			{
				outError = std::format(
					"{}: ссылка на GUID '{}' ожидает категорию {}, но указывает на {} ('{}'{})",
					context, guidStr, ToString(expectedKind), ToString(it->second.kind),
					it->second.sourcePath.string(),
					it->second.jsonPath.empty() ? "" : std::format(" {}", it->second.jsonPath));
				return false;
			}

			return true;
		}

		bool ValidateGuidStringField(
			const std::unordered_map<Guid, GuidOwnerInfo>& registry,
			const json& j,
			const char* key,
			GuidOwnerKind expectedKind,
			std::string_view context,
			std::string& outError,
			bool allowEmpty = false)
		{
			if (!j.contains(key))
				return true;

			const auto& val = j[key];
			if (!val.is_string())
			{
				outError = std::format("{}: поле '{}' должно быть строкой GUID.", context, key);
				return false;
			}

			return ValidateGuidRef(registry, val.get<std::string>(), expectedKind,
				std::format("{} (поле '{}')", context, key), outError, allowEmpty);
		}

		bool ValidateGuidArrayField(
			const std::unordered_map<Guid, GuidOwnerInfo>& registry,
			const json& j,
			const char* key,
			GuidOwnerKind expectedKind,
			std::string_view context,
			std::string& outError)
		{
			if (!j.contains(key))
				return true;

			const auto& arr = j[key];
			if (!arr.is_array())
			{
				outError = std::format("{}: поле '{}' должно быть массивом.", context, key);
				return false;
			}

			for (size_t i = 0; i < arr.size(); ++i)
			{
				const auto& elem = arr[i];
				if (!elem.is_string())
				{
					outError = std::format("{}: элемент '{}[{}]' должен быть строкой GUID.", context, key, i);
					return false;
				}

				const std::string& elemStr = elem.get<std::string>();
				if (elemStr.empty())
				{
					outError = std::format("{}: элемент '{}[{}]' содержит пустую строку вместо GUID.", context, key, i);
					return false;
				}

				if (!ValidateGuidRef(registry, elemStr, expectedKind,
					std::format("{} ({}[{}])", context, key, i), outError, /*allowEmpty=*/false))
				{
					return false;
				}
			}

			return true;
		}

		bool ValidateObjectsReferencesRecursively(
			const std::unordered_map<Guid, GuidOwnerInfo>& registry,
			const json& objectsArray,
			const fs::path& sceneRelPath,
			const std::string& parentJsonPath,
			std::string& outError)
		{
			if (!objectsArray.is_array())
			{
				outError = std::format("Сцена '{}': элемент '{}' должен быть массивом JSON.",
					sceneRelPath.string(), parentJsonPath);
				return false;
			}

			for (size_t i = 0; i < objectsArray.size(); ++i)
			{
				const auto& objElem = objectsArray[i];
				std::string currentPath = std::format("{}[{}]", parentJsonPath, i);

				if (!objElem.is_object())
				{
					outError = std::format("Сцена '{}': элемент '{}' не является объектом JSON.",
						sceneRelPath.string(), currentPath);
					return false;
				}

				std::string objName = objElem.value("name", "Object");
				std::string ctx = std::format("Сцена '{}', объект {} ('{}')", sceneRelPath.string(), currentPath, objName);

				// scripts / script
				if (!ValidateGuidStringField(registry, objElem, "script", GuidOwnerKind::Script, ctx, outError))
					return false;
				if (!ValidateGuidArrayField(registry, objElem, "scripts", GuidOwnerKind::Script, ctx, outError))
					return false;

				// render
				if (objElem.contains("render"))
				{
					const auto& render = objElem["render"];
					if (!render.is_object())
					{
						outError = std::format("{}: поле 'render' должно быть объектом JSON.", ctx);
						return false;
					}

					if (!ValidateGuidStringField(registry, render, "mesh", GuidOwnerKind::Mesh, ctx + " (render)", outError))
						return false;
					if (!ValidateGuidArrayField(registry, render, "submeshes", GuidOwnerKind::Mesh, ctx + " (render)", outError))
						return false;
					if (!ValidateGuidStringField(registry, render, "material", GuidOwnerKind::Material, ctx + " (render)", outError))
						return false;
					if (!ValidateGuidArrayField(registry, render, "materials", GuidOwnerKind::Material, ctx + " (render)", outError))
						return false;

					if (render.contains("parts"))
					{
						const auto& parts = render["parts"];
						if (!parts.is_array())
						{
							outError = std::format("{}: поле 'render.parts' должно быть массивом.", ctx);
							return false;
						}
						for (size_t pIdx = 0; pIdx < parts.size(); ++pIdx)
						{
							const auto& partElem = parts[pIdx];
							std::string partCtx = std::format("{} (render.parts[{}])", ctx, pIdx);
							if (!partElem.is_object())
							{
								outError = std::format("{}: элемент render.parts[{}] не является объектом JSON.", ctx, pIdx);
								return false;
							}
							if (!ValidateGuidStringField(registry, partElem, "mesh", GuidOwnerKind::Mesh, partCtx, outError))
								return false;
							if (!ValidateGuidStringField(registry, partElem, "material", GuidOwnerKind::Material, partCtx, outError))
								return false;
						}
					}
				}

				// prefab (опциональная ссылка, пустая строка допускается)
				if (!ValidateGuidStringField(registry, objElem, "prefab", GuidOwnerKind::Prefab, ctx, outError, /*allowEmpty=*/true))
					return false;

				if (objElem.contains("children"))
				{
					if (!objElem["children"].is_array())
					{
						outError = std::format("{}: поле 'children' должно быть массивом.", ctx);
						return false;
					}

					if (!ValidateObjectsReferencesRecursively(registry, objElem["children"], sceneRelPath, currentPath + ".children", outError))
						return false;
				}
			}

			return true;
		}

		bool ValidatePlatformConfigFile(
			const fs::path& configPath,
			const fs::path& projectDir,
			const std::unordered_map<Guid, GuidOwnerInfo>& registry,
			std::string& outError)
		{
			fs::path relPath = fs::relative(configPath, projectDir);
			std::string ctx = std::format("Платформенный конфиг '{}'", relPath.string());

			std::ifstream cf(configPath);
			if (!cf.is_open())
			{
				outError = std::format("{}: не удалось открыть файл конфигурации.", ctx);
				return false;
			}

			json configJson = json::parse(cf, nullptr, false);
			if (configJson.is_discarded())
			{
				outError = std::format("{}: содержит некорректный JSON синтаксис.", ctx);
				return false;
			}
			if (!configJson.is_object())
			{
				outError = std::format("{}: корневой элемент должен быть JSON-объектом.", ctx);
				return false;
			}

			auto validatePlatformBlock = [&](const json& block, const std::string& blockCtx) -> bool
			{
				// Скриптовые дельты
				if (!ValidateGuidArrayField(registry, block, "add_scripts", GuidOwnerKind::Script, blockCtx, outError))
					return false;
				if (!ValidateGuidArrayField(registry, block, "remove_scripts", GuidOwnerKind::Script, blockCtx, outError))
					return false;

				// Сценические дельты
				if (!ValidateGuidArrayField(registry, block, "add_scenes", GuidOwnerKind::Scene, blockCtx, outError))
					return false;
				if (!ValidateGuidArrayField(registry, block, "remove_scenes", GuidOwnerKind::Scene, blockCtx, outError))
					return false;

				// Стартовая сцена и стартовое вью
				if (!ValidateGuidStringField(registry, block, "start_scene", GuidOwnerKind::Scene, blockCtx, outError, false))
					return false;
				if (!ValidateGuidStringField(registry, block, "start_view", GuidOwnerKind::View, blockCtx, outError, false))
					return false;

				// Списки вторичных окон
				if (!ValidateGuidArrayField(registry, block, "child_views", GuidOwnerKind::View, blockCtx, outError))
					return false;
				if (!ValidateGuidArrayField(registry, block, "independent_views", GuidOwnerKind::View, blockCtx, outError))
					return false;

				// startView
				if (block.contains("startView"))
				{
					const auto& sv = block["startView"];
					if (!sv.is_object())
					{
						outError = std::format("{}: поле 'startView' должно быть JSON-объектом.", blockCtx);
						return false;
					}
					std::string svCtx = blockCtx + " (startView)";
					if (!ValidateGuidStringField(registry, sv, "scene", GuidOwnerKind::Scene, svCtx, outError, false))
						return false;
					if (!ValidateGuidStringField(registry, sv, "start_scene", GuidOwnerKind::Scene, svCtx, outError, false))
						return false;
					if (!ValidateGuidArrayField(registry, sv, "scripts", GuidOwnerKind::Script, svCtx, outError))
						return false;
					if (!ValidateGuidStringField(registry, sv, "script", GuidOwnerKind::Script, svCtx, outError, false))
						return false;
					if (!ValidateGuidStringField(registry, sv, "view", GuidOwnerKind::View, svCtx, outError, false))
						return false;
					if (!ValidateGuidStringField(registry, sv, "start_view", GuidOwnerKind::View, svCtx, outError, false))
						return false;
				}

				return true;
			};

			if (!validatePlatformBlock(configJson, ctx))
				return false;

			if (configJson.contains("platform"))
			{
				const auto& plat = configJson["platform"];
				if (!plat.is_object())
				{
					outError = std::format("{}: поле 'platform' должно быть JSON-объектом.", ctx);
					return false;
				}
				if (!validatePlatformBlock(plat, ctx + " (секция 'platform')"))
					return false;
			}

			return true;
		}
	}

	bool ProjectIdentityValidator::Validate(
		const fs::path& projectDir,
		char* errorBuffer,
		uint32_t bufferSize,
		const std::string& platformConfigFile) noexcept
	{
		try
		{
			if (errorBuffer && bufferSize > 0)
				errorBuffer[0] = '\0';

			std::string outError;
			std::unordered_map<Guid, GuidOwnerInfo> registry;

			// ----------------------------------------------------
			// ПРОХОД 1. Регистрация всех владельцев GUID
			// ----------------------------------------------------

			// 1.1. project.json и project.json.meta
			fs::path projJsonPath = projectDir / "project.json";
			fs::path projMetaPath = projectDir / "project.json.meta";

			if (!fs::exists(projJsonPath))
			{
				outError = "В корне проекта отсутствует обязательный файл 'project.json'.";
				SetError(errorBuffer, bufferSize, outError);
				return false;
			}

			{
				std::ifstream f(projJsonPath);
				json projJson = json::parse(f, nullptr, false);
				if (projJson.is_discarded())
				{
					outError = "Файл 'project.json' содержит некорректный JSON синтаксис.";
					SetError(errorBuffer, bufferSize, outError);
					return false;
				}
				if (!projJson.is_object())
				{
					outError = "Файл 'project.json' должен быть JSON-объектом.";
					SetError(errorBuffer, bufferSize, outError);
					return false;
				}
			}

			if (!fs::exists(projMetaPath))
			{
				outError = "В корне проекта отсутствует обязательный мета-файл 'project.json.meta'.";
				SetError(errorBuffer, bufferSize, outError);
				return false;
			}

			{
				std::ifstream f(projMetaPath);
				json metaJson = json::parse(f, nullptr, false);
				if (metaJson.is_discarded())
				{
					outError = "Файл 'project.json.meta' содержит некорректный JSON синтаксис.";
					SetError(errorBuffer, bufferSize, outError);
					return false;
				}
				if (!metaJson.is_object())
				{
					outError = "Файл 'project.json.meta' должен быть JSON-объектом.";
					SetError(errorBuffer, bufferSize, outError);
					return false;
				}
				if (!metaJson.contains("guid") || !metaJson["guid"].is_string())
				{
					outError = "Файл 'project.json.meta' повреждён или не содержит строковое поле 'guid'.";
					SetError(errorBuffer, bufferSize, outError);
					return false;
				}
				auto parsed = Guid::Parse(metaJson["guid"].get<std::string>());
				if (!parsed)
				{
					outError = std::format("Файл 'project.json.meta' содержит некорректный GUID '{}'.", metaJson["guid"].get<std::string>());
					SetError(errorBuffer, bufferSize, outError);
					return false;
				}
				if (!RegisterGuid(registry, *parsed, GuidOwnerKind::Project, "project.json", "", outError))
				{
					SetError(errorBuffer, bufferSize, outError);
					return false;
				}
			}

			// 1.2. Рекурсивное сканирование каталога Assets/
			fs::path assetsDir = projectDir / "Assets";
			if (fs::exists(assetsDir) && fs::is_directory(assetsDir))
			{
				for (const auto& entry : fs::recursive_directory_iterator(assetsDir))
				{
					if (!entry.is_regular_file())
						continue;

					fs::path path = entry.path();
					std::string ext = path.extension().string();
					if (ext == ".meta")
						continue;

					fs::path relPath = fs::relative(path, projectDir);
					GuidOwnerKind kind = GuidOwnerKind::Mesh;
					bool isKnownAsset = false;

					if (ext == c_ExtScene)
					{
						kind = GuidOwnerKind::Scene;
						isKnownAsset = true;
					}
					else if (ext == c_ExtView)
					{
						kind = GuidOwnerKind::View;
						isKnownAsset = true;
					}
					else if (ext == c_ExtMeshObj)
					{
						kind = GuidOwnerKind::Mesh;
						isKnownAsset = true;
					}
					else if (ext == c_ExtMaterial)
					{
						kind = GuidOwnerKind::Material;
						isKnownAsset = true;
					}
					else if (ext == c_ExtShaderHlsl)
					{
						kind = GuidOwnerKind::Shader;
						isKnownAsset = true;
					}
					else if (ext == c_ExtTexturePng)
					{
						kind = GuidOwnerKind::Texture;
						isKnownAsset = true;
					}
					else if (ext == c_ExtPrefab)
					{
						kind = GuidOwnerKind::Prefab;
						isKnownAsset = true;
					}
					else if (ext == ".h" || ext == ".hpp")
					{
						std::string genericRel = relPath.generic_string();
						if (genericRel.find("Assets/Scripts/") != std::string::npos ||
							genericRel.find("Assets/scripts/") != std::string::npos)
						{
							kind = GuidOwnerKind::Script;
							isKnownAsset = true;
						}
					}

					if (!isKnownAsset)
						continue;

					fs::path metaPath = path;
					metaPath += ".meta";
					if (!fs::exists(metaPath))
					{
						outError = std::format("Отсутствует обязательный мета-файл '{}' для ассета '{}'.",
							metaPath.filename().string(), relPath.string());
						SetError(errorBuffer, bufferSize, outError);
						return false;
					}

					std::ifstream mf(metaPath);
					json metaJson = json::parse(mf, nullptr, false);
					if (metaJson.is_discarded())
					{
						outError = std::format("Мета-файл '{}' содержит некорректный JSON синтаксис.",
							fs::relative(metaPath, projectDir).string());
						SetError(errorBuffer, bufferSize, outError);
						return false;
					}
					if (!metaJson.is_object() || !metaJson.contains("guid") || !metaJson["guid"].is_string())
					{
						outError = std::format("Мета-файл '{}' повреждён или не содержит строковое поле 'guid'.",
							fs::relative(metaPath, projectDir).string());
						SetError(errorBuffer, bufferSize, outError);
						return false;
					}

					auto parsed = Guid::Parse(metaJson["guid"].get<std::string>());
					if (!parsed)
					{
						outError = std::format("Мета-файл '{}' содержит некорректный GUID '{}'.",
							fs::relative(metaPath, projectDir).string(), metaJson["guid"].get<std::string>());
						SetError(errorBuffer, bufferSize, outError);
						return false;
					}

					if (!RegisterGuid(registry, *parsed, kind, relPath, "", outError))
					{
						SetError(errorBuffer, bufferSize, outError);
						return false;
					}

					// Проверка синтаксиса представления (.zv)
					if (kind == GuidOwnerKind::View)
					{
						std::ifstream vf(path);
						json viewJson = json::parse(vf, nullptr, false);
						if (viewJson.is_discarded())
						{
							outError = std::format("Представление '{}' содержит некорректный JSON синтаксис.", relPath.string());
							SetError(errorBuffer, bufferSize, outError);
							return false;
						}
						if (!viewJson.is_object())
						{
							outError = std::format("Представление '{}' должно быть JSON-объектом.", relPath.string());
							SetError(errorBuffer, bufferSize, outError);
							return false;
						}
					}

					// Если это сцена (.zs), регистрируем слои и объекты
					if (kind == GuidOwnerKind::Scene)
					{
						std::ifstream sf(path);
						json sceneJson = json::parse(sf, nullptr, false);
						if (sceneJson.is_discarded())
						{
							outError = std::format("Ошибка синтаксиса JSON в сцене '{}'.", relPath.string());
							SetError(errorBuffer, bufferSize, outError);
							return false;
						}
						if (!sceneJson.is_object())
						{
							outError = std::format("Сцена '{}' должна быть JSON-объектом.", relPath.string());
							SetError(errorBuffer, bufferSize, outError);
							return false;
						}

						if (sceneJson.contains("objects"))
						{
							outError = std::format(
								"Сцена '{}' содержит устаревший корневой массив 'objects'. Объекты должны находиться строго внутри layers[].objects.",
								relPath.string());
							SetError(errorBuffer, bufferSize, outError);
							return false;
						}

						if (sceneJson.contains("layers"))
						{
							if (!sceneJson["layers"].is_array())
							{
								outError = std::format("Сцена '{}': поле 'layers' должно быть массивом.", relPath.string());
								SetError(errorBuffer, bufferSize, outError);
								return false;
							}

							const auto& layers = sceneJson["layers"];
							for (size_t lIdx = 0; lIdx < layers.size(); ++lIdx)
							{
								const auto& layerElem = layers[lIdx];
								if (!layerElem.is_object())
								{
									outError = std::format("Сцена '{}': элемент layers[{}] не является объектом JSON.",
										relPath.string(), lIdx);
									SetError(errorBuffer, bufferSize, outError);
									return false;
								}

								if (layerElem.contains("name") && !layerElem["name"].is_string())
								{
									outError = std::format("Сцена '{}', слой layers[{}]: поле 'name' должно быть строкой.",
										relPath.string(), lIdx);
									SetError(errorBuffer, bufferSize, outError);
									return false;
								}

								std::string layerName = layerElem.value("name", "Layer");
								std::string layerPath = std::format("layers[{}]", lIdx);

								if (!layerElem.contains("guid") || !layerElem["guid"].is_string())
								{
									outError = std::format("Сцена '{}', слой {} ('{}'): отсутствует обязательное строковое поле 'guid'.",
										relPath.string(), layerPath, layerName);
									SetError(errorBuffer, bufferSize, outError);
									return false;
								}

								auto layerGuid = Guid::Parse(layerElem["guid"].get<std::string>());
								if (!layerGuid)
								{
									outError = std::format("Сцена '{}', слой {} ('{}'): некорректный GUID '{}'.",
										relPath.string(), layerPath, layerName, layerElem["guid"].get<std::string>());
									SetError(errorBuffer, bufferSize, outError);
									return false;
								}

								if (!RegisterGuid(registry, *layerGuid, GuidOwnerKind::Layer, relPath, layerPath, outError))
								{
									SetError(errorBuffer, bufferSize, outError);
									return false;
								}

								if (layerElem.contains("objects"))
								{
									if (!layerElem["objects"].is_array())
									{
										outError = std::format("Сцена '{}', слой {} ('{}'): поле 'objects' должно быть массивом.",
											relPath.string(), layerPath, layerName);
										SetError(errorBuffer, bufferSize, outError);
										return false;
									}

									if (!RegisterObjectsRecursively(registry, layerElem["objects"], relPath, layerPath + ".objects", outError))
									{
										SetError(errorBuffer, bufferSize, outError);
										return false;
									}
								}
							}
						}
					}
				}
			}

			// ----------------------------------------------------
			// ПРОХОД 2. Проверка типизированных GUID-ссылок
			// ----------------------------------------------------

			// 2.1. project.json
			std::ifstream f(projJsonPath);
			json projJson = json::parse(f, nullptr, false);
			if (projJson.is_discarded())
			{
				outError = "Файл 'project.json' содержит некорректный JSON синтаксис.";
				SetError(errorBuffer, bufferSize, outError);
				return false;
			}
			if (!projJson.is_object())
			{
				outError = "Файл 'project.json' должен быть JSON-объектом.";
				SetError(errorBuffer, bufferSize, outError);
				return false;
			}

			std::string projCtx = "project.json";
			if (!ValidateGuidStringField(registry, projJson, "start_scene", GuidOwnerKind::Scene, projCtx, outError))
			{
				SetError(errorBuffer, bufferSize, outError);
				return false;
			}
			if (!ValidateGuidStringField(registry, projJson, "start_view", GuidOwnerKind::View, projCtx, outError))
			{
				SetError(errorBuffer, bufferSize, outError);
				return false;
			}
			if (!ValidateGuidArrayField(registry, projJson, "scenes", GuidOwnerKind::Scene, projCtx, outError))
			{
				SetError(errorBuffer, bufferSize, outError);
				return false;
			}
			if (!ValidateGuidArrayField(registry, projJson, "views", GuidOwnerKind::View, projCtx, outError))
			{
				SetError(errorBuffer, bufferSize, outError);
				return false;
			}
			if (!ValidateGuidArrayField(registry, projJson, "game_scripts", GuidOwnerKind::Script, projCtx, outError))
			{
				SetError(errorBuffer, bufferSize, outError);
				return false;
			}
			if (!ValidateGuidStringField(registry, projJson, "game_script", GuidOwnerKind::Script, projCtx, outError))
			{
				SetError(errorBuffer, bufferSize, outError);
				return false;
			}

			// 2.2. Платформенные конфигурации (platformConfigFile, пресеты targets[].config_file, build_settings/platforms/**/*.json)
			std::vector<fs::path> configsToValidate;
			std::unordered_set<std::string> seenConfigPaths;

			auto addConfigIfValid = [&](const fs::path& p) {
				std::error_code ec;
				if (fs::exists(p, ec) && fs::is_regular_file(p, ec))
				{
					fs::path canonicalP = fs::weakly_canonical(p, ec);
					std::string pathKey = canonicalP.string();
					if (seenConfigPaths.insert(pathKey).second)
					{
						configsToValidate.push_back(canonicalP);
					}
				}
			};

			// 2.2.1. Конкретный platformConfigFile, если передан
			if (!platformConfigFile.empty())
			{
				fs::path p(platformConfigFile);
				fs::path candidate = p.is_absolute() ? p : (projectDir / p);
				if (!fs::exists(candidate))
				{
					candidate = projectDir / "build_settings" / p;
				}
				if (!fs::exists(candidate))
				{
					outError = std::format("Указанный платформенный конфигурационный файл не найден: '{}'", platformConfigFile);
					SetError(errorBuffer, bufferSize, outError);
					return false;
				}
				addConfigIfValid(candidate);
			}

			// 2.2.2. Пресеты (presets_file из project.json или build_settings/presets.json)
			{
				fs::path presetsPath;
				if (projJson.contains("presets_file") && projJson["presets_file"].is_string())
				{
					presetsPath = projectDir / projJson["presets_file"].get<std::string>();
				}
				else if (projJson.contains("build_settings") && projJson["build_settings"].is_object()
					&& projJson["build_settings"].contains("presets_file") && projJson["build_settings"]["presets_file"].is_string())
				{
					presetsPath = projectDir / projJson["build_settings"]["presets_file"].get<std::string>();
				}
				else
				{
					presetsPath = projectDir / "build_settings" / "presets.json";
				}

				if (fs::exists(presetsPath) && fs::is_regular_file(presetsPath))
				{
					std::ifstream pf(presetsPath);
					json presetsJson = json::parse(pf, nullptr, false);
					if (!presetsJson.is_discarded() && presetsJson.is_object() && presetsJson.contains("presets") && presetsJson["presets"].is_array())
					{
						for (const auto& presetElem : presetsJson["presets"])
						{
							if (presetElem.is_object() && presetElem.contains("targets") && presetElem["targets"].is_array())
							{
								for (const auto& targetElem : presetElem["targets"])
								{
									if (targetElem.is_object() && targetElem.contains("config_file") && targetElem["config_file"].is_string())
									{
										std::string cfgStr = targetElem["config_file"].get<std::string>();
										fs::path cp(cfgStr);
										fs::path fullCp = cp.is_absolute() ? cp : (projectDir / cp);
										if (!fs::exists(fullCp))
											fullCp = projectDir / "build_settings" / cp;
										addConfigIfValid(fullCp);
									}
								}
							}
						}
					}
				}
			}

			// 2.2.3. Каталог build_settings/platforms/
			fs::path platformsDir = projectDir / "build_settings" / "platforms";
			if (fs::exists(platformsDir) && fs::is_directory(platformsDir))
			{
				for (const auto& entry : fs::recursive_directory_iterator(platformsDir))
				{
					if (entry.is_regular_file() && entry.path().extension() == ".json")
					{
						addConfigIfValid(entry.path());
					}
				}
			}

			// 2.2.4. Валидация всех собранных конфигов
			for (const auto& cfgPath : configsToValidate)
			{
				if (!ValidatePlatformConfigFile(cfgPath, projectDir, registry, outError))
				{
					SetError(errorBuffer, bufferSize, outError);
					return false;
				}
			}

			// 2.3. Ссылки внутри ассетов (.zv и .zs)
			if (fs::exists(assetsDir) && fs::is_directory(assetsDir))
			{
				for (const auto& entry : fs::recursive_directory_iterator(assetsDir))
				{
					if (!entry.is_regular_file())
						continue;

					fs::path path = entry.path();
					std::string ext = path.extension().string();
					fs::path relPath = fs::relative(path, projectDir);

					if (ext == c_ExtView)
					{
						std::ifstream vf(path);
						json viewJson = json::parse(vf, nullptr, false);
						if (viewJson.is_discarded())
						{
							outError = std::format("Представление '{}' содержит некорректный JSON синтаксис.", relPath.string());
							SetError(errorBuffer, bufferSize, outError);
							return false;
						}
						if (!viewJson.is_object())
						{
							outError = std::format("Представление '{}' должно быть JSON-объектом.", relPath.string());
							SetError(errorBuffer, bufferSize, outError);
							return false;
						}

						std::string ctx = std::format("Представление '{}'", relPath.string());
						if (!ValidateGuidStringField(registry, viewJson, "scene", GuidOwnerKind::Scene, ctx, outError))
						{
							SetError(errorBuffer, bufferSize, outError);
							return false;
						}
						if (!ValidateGuidStringField(registry, viewJson, "script", GuidOwnerKind::Script, ctx, outError))
						{
							SetError(errorBuffer, bufferSize, outError);
							return false;
						}
						if (!ValidateGuidArrayField(registry, viewJson, "scripts", GuidOwnerKind::Script, ctx, outError))
						{
							SetError(errorBuffer, bufferSize, outError);
							return false;
						}

						if (viewJson.contains("startView"))
						{
							const auto& sv = viewJson["startView"];
							if (!sv.is_object())
							{
								outError = std::format("{}: поле 'startView' должно быть JSON-объектом.", ctx);
								SetError(errorBuffer, bufferSize, outError);
								return false;
							}
							std::string svCtx = ctx + " (startView)";
							if (!ValidateGuidStringField(registry, sv, "scene", GuidOwnerKind::Scene, svCtx, outError))
							{
								SetError(errorBuffer, bufferSize, outError);
								return false;
							}
							if (!ValidateGuidStringField(registry, sv, "script", GuidOwnerKind::Script, svCtx, outError))
							{
								SetError(errorBuffer, bufferSize, outError);
								return false;
							}
							if (!ValidateGuidArrayField(registry, sv, "scripts", GuidOwnerKind::Script, svCtx, outError))
							{
								SetError(errorBuffer, bufferSize, outError);
								return false;
							}
						}
					}
					else if (ext == c_ExtScene)
					{
						std::ifstream sf(path);
						json sceneJson = json::parse(sf, nullptr, false);
						if (sceneJson.is_discarded())
						{
							outError = std::format("Сцена '{}' содержит некорректный JSON синтаксис.", relPath.string());
							SetError(errorBuffer, bufferSize, outError);
							return false;
						}
						if (!sceneJson.is_object())
						{
							outError = std::format("Сцена '{}' должна быть JSON-объектом.", relPath.string());
							SetError(errorBuffer, bufferSize, outError);
							return false;
						}

						std::string ctx = std::format("Сцена '{}'", relPath.string());
						if (!ValidateGuidStringField(registry, sceneJson, "script", GuidOwnerKind::Script, ctx, outError))
						{
							SetError(errorBuffer, bufferSize, outError);
							return false;
						}
						if (!ValidateGuidArrayField(registry, sceneJson, "scripts", GuidOwnerKind::Script, ctx, outError))
						{
							SetError(errorBuffer, bufferSize, outError);
							return false;
						}

						if (sceneJson.contains("layers"))
						{
							const auto& layers = sceneJson["layers"];
							if (!layers.is_array())
							{
								outError = std::format("{}: поле 'layers' должно быть массивом.", ctx);
								SetError(errorBuffer, bufferSize, outError);
								return false;
							}

							for (size_t lIdx = 0; lIdx < layers.size(); ++lIdx)
							{
								const auto& layerElem = layers[lIdx];
								if (!layerElem.is_object())
								{
									outError = std::format("{}: элемент layers[{}] должен быть JSON-объектом.", ctx, lIdx);
									SetError(errorBuffer, bufferSize, outError);
									return false;
								}

								std::string layerPath = std::format("layers[{}]", lIdx);
								if (layerElem.contains("objects"))
								{
									if (!layerElem["objects"].is_array())
									{
										outError = std::format("{}: поле '{}.objects' должно быть массивом.", ctx, layerPath);
										SetError(errorBuffer, bufferSize, outError);
										return false;
									}

									if (!ValidateObjectsReferencesRecursively(registry, layerElem["objects"], relPath, layerPath + ".objects", outError))
									{
										SetError(errorBuffer, bufferSize, outError);
										return false;
									}
								}
							}
						}
					}
				}
			}

			return true;
		}
		catch (const std::exception& ex)
		{
			SetError(errorBuffer, bufferSize, std::format("Исключение при валидации проекта: {}", ex.what()));
			return false;
		}
		catch (...)
		{
			SetError(errorBuffer, bufferSize, "Неизвестное исключение при валидации проекта.");
			return false;
		}
	}
}
