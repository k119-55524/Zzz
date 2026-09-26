#pragma once

#include <filesystem>
#include <string>
#include <core/enums/eTargetPlatform.h>
#include "StageValidationResult.h"
#include "StagePackPlan.h"

namespace zzz::builder
{
	/**
	 * @class Stage2_PackPlanner
	 * @brief Стадия 2 модульного конвейера сборщика (индивидуальна для каждого таргета).
	 *
	 * Раскручивает граф зависимостей от активных сцен и вьюх, выполняет dead-code stripping,
	 * зондирует текстуры через TextureBuilder::Probe и логирует сводную таблицу ресурсов.
	 */
	class Stage2_PackPlanner
	{
	public:
		[[nodiscard]] static StagePackPlan Plan(
			const StageValidationResult& valResult,
			const std::filesystem::path& sourceDir,
			const std::filesystem::path& destinationDir,
			core::eTargetPlatform targetPlatform,
			const std::string& platformConfigFile = "",
			uint64_t buildTimestamp = 0);
	};
}
