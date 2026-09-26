#pragma once

#include <map>
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_set>
#include <core/Core.h>
#include <core/utils/Guid.h>
#include <core/enums/eTargetPlatform.h>
#include <core/enums/eDataDatType.h>
#include <core/io/package/assets/AssetMetadata.h>

#include "StageValidationResult.h"

namespace zzz::builder
{
	/**
	 * @brief Результат предварительного зондирования (probe) текстуры.
	 */
	struct ProbedTextureInfo
	{
		zzz::core::Guid            guid{};
		std::string                relativePath;
		zzz::core::TextureMetadata metadata{};
		std::string                sourceFormatName;
		std::string                targetFormatName;
		bool                       success{ false };
		std::string                errorMessage;
	};

	/**
	 * @brief DTO-план упаковки таргета (выход Стадии 2, вход Стадии 3).
	 */
	struct StagePackPlan
	{
		bool                                               isValid{ false };
		std::string                                        errorMessage;

		// Параметры таргета
		std::filesystem::path                              sourceDir;
		std::filesystem::path                              destinationDir;
		zzz::core::eTargetPlatform                         targetPlatform{ zzz::core::eTargetPlatform::Windows };
		std::string                                        platformConfigFile;
		uint64_t                                           buildTimestamp{ 0 };

		// Активные структурные ресурсы package.dat
		std::vector<std::string>                           activeSceneNames;
		std::vector<ProjectAssetInfo>                      activeSceneAssets;
		std::vector<ProjectAssetInfo>                      activeViewAssets;


		// Корзины данных по eDataDatType (Mesh, Material, Shader -> inline; Texture2D, AudioClip -> external)
		std::map<zzz::core::eDataDatType, std::vector<ProjectAssetInfo>> assetsByPak;

		// Отсеянные data-ресурсы (dead-code elimination)
		std::vector<ProjectAssetInfo>                      strippedAssets;

		// Результаты Probe для текстур
		std::unordered_map<zzz::core::Guid, ProbedTextureInfo> probedTextures;
	};
}
