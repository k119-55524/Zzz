#pragma once

#include <vector>
#include <string>
#include <optional>
#include <filesystem>
#include <unordered_map>
#include <core/Core.h>
#include <core/utils/Guid.h>
#include <core/enums/eEngineResourceType.h>
#include <core/enums/eDataDatType.h>
#include "json.hpp"

namespace zzz::builder
{
	/**
	 * @brief Информация об обнаруженном ассете проекта.
	 */
	struct ProjectAssetInfo
	{
		zzz::core::Guid                  guid{};
		std::filesystem::path            relativePath;  // Относительно каталога Assets/
		std::filesystem::path            fullPath;      // Полный абсолютный путь к файлу ресурса
		std::filesystem::path            metaPath;      // Полный путь к файлу .meta
		zzz::core::eEngineResourceType   resourceType{ zzz::core::eEngineResourceType::Unknown };
		std::optional<zzz::core::eDataDatType> dataDatType;
		std::filesystem::file_time_type  lastWriteTime{};
		uintmax_t                        fileSize{ 0 };
		std::filesystem::file_time_type  metaLastWriteTime{};
		uintmax_t                        metaFileSize{ 0 };
		nlohmann::json                   metaJson;

		[[nodiscard]] bool IsInlineData() const noexcept
		{
			if (!dataDatType.has_value())
				return false;
			switch (*dataDatType)
			{
			case zzz::core::eDataDatType::Mesh:
			case zzz::core::eDataDatType::Material:
			case zzz::core::eDataDatType::Shader:
			case zzz::core::eDataDatType::Animation:
				return true;
			default:
				return false;
			}
		}

		[[nodiscard]] bool IsExternalPak() const noexcept
		{
			if (!dataDatType.has_value())
				return false;
			return !IsInlineData();
		}
	};

	/**
	 * @brief Запись о состоянии файла в момент снимка (snapshot).
	 */
	struct FileSnapshot
	{
		std::filesystem::file_time_type lastWriteTime{};
		uintmax_t                        fileSize{ 0 };
	};

	/**
	 * @brief DTO-результат выполнения Стадии 1 (валидация проекта, общая на всю сессию).
	 */
	struct StageValidationResult
	{
		bool                                    isValid{ false };
		std::string                             errorMessage;
		std::filesystem::path                   projectDir;
		std::filesystem::path                   assetsDir;
		std::vector<ProjectAssetInfo>           allAssets;
		std::unordered_map<zzz::core::Guid, std::size_t> guidToIndex;
		std::vector<std::string>                discoveredScenes; // Относительные пути сцен внутри Assets/
		std::vector<std::string>                discoveredViews;  // Относительные пути вьюх внутри Assets/

		std::filesystem::path                   projectJsonPath;
		FileSnapshot                            projectJsonSnapshot;
		std::filesystem::path                   projectMetaPath;
		bool                                    hasProjectMeta{ false };
		FileSnapshot                            projectMetaSnapshot;
		std::unordered_map<std::string, FileSnapshot> platformConfigSnapshots;

		[[nodiscard]] const ProjectAssetInfo* FindAsset(const zzz::core::Guid& guid) const noexcept
		{
			const auto it = guidToIndex.find(guid);
			if (it != guidToIndex.end() && it->second < allAssets.size())
			{
				return &allAssets[it->second];
			}
			return nullptr;
		}

		/**
		 * @brief Проверяет snapshot проекта на предмет изменения файлов внешним процессом.
		 */
		[[nodiscard]] bool CheckSnapshotConsistent(std::string* outError = nullptr) const
		{
			std::error_code ec;

			// 1. Проверка файлов ресурсов и их .meta
			for (const auto& asset : allAssets)
			{
				if (!std::filesystem::exists(asset.fullPath, ec))
				{
					if (outError)
						*outError = "Файл ресурса удалён во время сессии сборки: " + asset.relativePath.string();
					return false;
				}
				const auto curTime = std::filesystem::last_write_time(asset.fullPath, ec);
				const auto curSize = std::filesystem::file_size(asset.fullPath, ec);
				if (curTime != asset.lastWriteTime || curSize != asset.fileSize)
				{
					if (outError)
						*outError = "Файл ресурса изменён во время сессии сборки: " + asset.relativePath.string() + 
						            ". Пожалуйста, начните новую сессию сборки.";
					return false;
				}

				if (!std::filesystem::exists(asset.metaPath, ec))
				{
					if (outError)
						*outError = "Файл метаданных (.meta) удалён во время сессии сборки: " + asset.metaPath.string();
					return false;
				}
				const auto curMetaTime = std::filesystem::last_write_time(asset.metaPath, ec);
				const auto curMetaSize = std::filesystem::file_size(asset.metaPath, ec);
				if (curMetaTime != asset.metaLastWriteTime || curMetaSize != asset.metaFileSize)
				{
					if (outError)
						*outError = "Файл метаданных (.meta) изменён во время сессии сборки: " + asset.metaPath.string() + 
						            ". Пожалуйста, начните новую сессию сборки.";
					return false;
				}
			}

			// 2. Проверка project.json
			if (!projectJsonPath.empty())
			{
				if (!std::filesystem::exists(projectJsonPath, ec))
				{
					if (outError)
						*outError = "Файл project.json удалён во время сессии сборки: " + projectJsonPath.string();
					return false;
				}
				const auto curTime = std::filesystem::last_write_time(projectJsonPath, ec);
				const auto curSize = std::filesystem::file_size(projectJsonPath, ec);
				if (curTime != projectJsonSnapshot.lastWriteTime || curSize != projectJsonSnapshot.fileSize)
				{
					if (outError)
						*outError = "Файл project.json изменён во время сессии сборки. Пожалуйста, начните новую сессию сборки.";
					return false;
				}
			}

			// 3. Проверка project.json.meta (если существовал)
			if (hasProjectMeta)
			{
				if (!std::filesystem::exists(projectMetaPath, ec))
				{
					if (outError)
						*outError = "Файл project.json.meta удалён во время сессии сборки: " + projectMetaPath.string();
					return false;
				}
				const auto curTime = std::filesystem::last_write_time(projectMetaPath, ec);
				const auto curSize = std::filesystem::file_size(projectMetaPath, ec);
				if (curTime != projectMetaSnapshot.lastWriteTime || curSize != projectMetaSnapshot.fileSize)
				{
					if (outError)
						*outError = "Файл project.json.meta изменён во время сессии сборки. Пожалуйста, начните новую сессию сборки.";
					return false;
				}
			}

			// 4. Проверка зафиксированных файлов конфигураций платформ
			for (const auto& [pathStr, snap] : platformConfigSnapshots)
			{
				std::filesystem::path cfgPath(pathStr);
				if (!std::filesystem::exists(cfgPath, ec))
				{
					if (outError)
						*outError = "Платформенный конфигурационный файл удалён во время сессии сборки: " + pathStr;
					return false;
				}
				const auto curTime = std::filesystem::last_write_time(cfgPath, ec);
				const auto curSize = std::filesystem::file_size(cfgPath, ec);
				if (curTime != snap.lastWriteTime || curSize != snap.fileSize)
				{
					if (outError)
						*outError = "Платформенный конфигурационный файл изменён во время сессии сборки: " + pathStr +
						            ". Пожалуйста, начните новую сессию сборки.";
					return false;
				}
			}

			return true;
		}

		/**
		 * @brief Проверяет конкретный конфигурационный файл платформы на соответствие snapshot.
		 */
		[[nodiscard]] bool CheckPlatformConfigFile(const std::filesystem::path& configPath, std::string* outError = nullptr) const
		{
			std::error_code ec;
			if (!std::filesystem::exists(configPath, ec))
			{
				if (outError)
					*outError = "Платформенный конфигурационный файл не найден: " + configPath.string();
				return false;
			}

			auto it = platformConfigSnapshots.find(configPath.string());
			if (it != platformConfigSnapshots.end())
			{
				const auto curTime = std::filesystem::last_write_time(configPath, ec);
				const auto curSize = std::filesystem::file_size(configPath, ec);
				if (curTime != it->second.lastWriteTime || curSize != it->second.fileSize)
				{
					if (outError)
						*outError = "Платформенный конфигурационный файл изменён во время сессии сборки: " + configPath.string() +
						            ". Пожалуйста, начните новую сессию сборки.";
					return false;
				}
			}
			return true;
		}
	};
}
