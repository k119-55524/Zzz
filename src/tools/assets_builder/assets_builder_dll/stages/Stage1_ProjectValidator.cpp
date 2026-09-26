#include "Stage1_ProjectValidator.h"
#include <fstream>
#include <format>
#include "../AssetScanner.h"
#include "../AssetImporterRegistry.h"
#include "../ProjectIdentityValidator.h"
#include "core/enums/ResourceDatMapping.h"
#include "core/logger/logger.h"

namespace zzz::builder
{
	StageValidationResult Stage1_ProjectValidator::Validate(const std::filesystem::path& projectDir)
	{
		StageValidationResult result;
		result.projectDir = projectDir;

		std::error_code ec;
		if (!std::filesystem::exists(projectDir, ec) || !std::filesystem::is_directory(projectDir, ec))
		{
			result.isValid = false;
			result.errorMessage = std::format("Каталог проекта не существует: {}", projectDir.string());
			return result;
		}

		// Сначала запускаем строгую проверку ссылочной целостности и GUID проекта
		char errorBuffer[2048] = { 0 };
		if (!ProjectIdentityValidator::Validate(projectDir, errorBuffer, sizeof(errorBuffer)))
		{
			result.isValid = false;
			result.errorMessage = errorBuffer[0] != '\0' ? errorBuffer : "Сбой валидации идентичности проекта (ProjectIdentityValidator)";
			return result;
		}

		const auto assetsDir = projectDir / "Assets";
		result.assetsDir = assetsDir;

		if (!std::filesystem::exists(assetsDir, ec))
		{
			// Пустой проект без Assets/ допустим
			result.isValid = true;
			return result;
		}

		bool scanOk = ScanAssetsDirectory(assetsDir, [&](const ScannedAssetFile& file) -> bool {
			const auto relPath = std::filesystem::relative(file.path, assetsDir);
			const auto metaPath = std::filesystem::path(file.path.string() + ".meta");

			if (!std::filesystem::exists(metaPath, ec))
			{
				result.errorMessage = std::format("Отсутствует обязательный .meta файл для: {}", relPath.string());
				return false;
			}

			nlohmann::json metaJson;
			{
				std::ifstream metaFile(metaPath);
				if (!metaFile.is_open())
				{
					result.errorMessage = std::format("Не удалось открыть .meta файл: {}", metaPath.string());
					return false;
				}
				try
				{
					metaFile >> metaJson;
				}
				catch (const std::exception& e)
				{
					result.errorMessage = std::format("Ошибка парсинга JSON в .meta файле {}: {}", metaPath.string(), e.what());
					return false;
				}
			}

			if (!metaJson.contains("guid") || !metaJson["guid"].is_string())
			{
				result.errorMessage = std::format("Поле 'guid' отсутствует или не является строкой в: {}", metaPath.string());
				return false;
			}

			const std::string guidStr = metaJson["guid"].get<std::string>();
			auto guidOpt = core::Guid::Parse(guidStr);
			if (!guidOpt.has_value() || guidOpt->IsEmpty())
			{
				result.errorMessage = std::format("Некорректный или пустой GUID '{}' в: {}", guidStr, metaPath.string());
				return false;
			}

			const core::Guid guid = *guidOpt;
			if (result.guidToIndex.contains(guid))
			{
				result.errorMessage = std::format("Обнаружен дубликат GUID '{}' для файла: {}", guidStr, relPath.string());
				return false;
			}

			auto resourceType = file.knownType.value_or(core::eEngineResourceType::Unknown);
			if (resourceType == core::eEngineResourceType::Unknown)
			{
				const auto known = AssetImporterRegistry::Instance().GetKnownType(file.extension);
				if (known.has_value())
					resourceType = *known;
			}

			ProjectAssetInfo info;
			info.guid = guid;
			info.relativePath = relPath;
			info.fullPath = file.path;
			info.metaPath = metaPath;
			info.resourceType = resourceType;
			info.dataDatType = core::TryToDataDatType(resourceType);
			info.lastWriteTime = std::filesystem::last_write_time(file.path, ec);
			info.fileSize = std::filesystem::file_size(file.path, ec);
			info.metaLastWriteTime = std::filesystem::last_write_time(metaPath, ec);
			info.metaFileSize = std::filesystem::file_size(metaPath, ec);
			info.metaJson = std::move(metaJson);

			const std::size_t newIndex = result.allAssets.size();
			result.allAssets.push_back(std::move(info));
			result.guidToIndex[guid] = newIndex;

			if (resourceType == core::eEngineResourceType::Scene)
			{
				result.discoveredScenes.push_back(relPath.generic_string());
			}
			else if (resourceType == core::eEngineResourceType::View)
			{
				result.discoveredViews.push_back(relPath.generic_string());
			}

			return true;
		});

		if (!scanOk)
		{
			result.isValid = false;
			if (result.errorMessage.empty())
				result.errorMessage = "Ошибка сканирования директории Assets";
			return result;
		}

		// Фиксация снимка project.json и project.json.meta
		const auto projectJson = projectDir / "project.json";
		if (std::filesystem::exists(projectJson, ec))
		{
			result.projectJsonPath = projectJson;
			result.projectJsonSnapshot.lastWriteTime = std::filesystem::last_write_time(projectJson, ec);
			result.projectJsonSnapshot.fileSize = std::filesystem::file_size(projectJson, ec);
		}

		const auto projectMeta = projectDir / "project.json.meta";
		if (std::filesystem::exists(projectMeta, ec))
		{
			result.hasProjectMeta = true;
			result.projectMetaPath = projectMeta;
			result.projectMetaSnapshot.lastWriteTime = std::filesystem::last_write_time(projectMeta, ec);
			result.projectMetaSnapshot.fileSize = std::filesystem::file_size(projectMeta, ec);
		}

		// Фиксация снимка конфигурационных файлов платформ в build_settings/
		const auto buildSettingsDir = projectDir / "build_settings";
		if (std::filesystem::exists(buildSettingsDir, ec) && std::filesystem::is_directory(buildSettingsDir, ec))
		{
			for (const auto& entry : std::filesystem::recursive_directory_iterator(buildSettingsDir, ec))
			{
				if (entry.is_regular_file(ec) && entry.path().extension() == ".json")
				{
					result.platformConfigSnapshots.emplace(
						entry.path().string(),
						FileSnapshot{
							std::filesystem::last_write_time(entry.path(), ec),
							std::filesystem::file_size(entry.path(), ec)
						});
				}
			}
		}

		result.isValid = true;
		return result;
	}
}
