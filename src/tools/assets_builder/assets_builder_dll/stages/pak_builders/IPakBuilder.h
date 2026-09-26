#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <core/io/package/PackageEntry.h>
#include "../StagePackPlan.h"

namespace zzz::builder
{
	/**
	 * @brief Результат сборки одного пакета или архива.
	 */
	struct PakBuildResult
	{
		bool                                  success{ false };
		std::string                           errorMessage;
		std::vector<zzz::core::PackageEntry>  entries;
		std::filesystem::path                 outputFilePath;
		uint64_t                              outputFileSize{ 0 };
	};

	/**
	 * @brief Базовый интерфейс стратегии сборки паков (Strategy pattern).
	 */
	class IPakBuilder
	{
	public:
		virtual ~IPakBuilder() = default;

		[[nodiscard]] virtual PakBuildResult Build(
			const StagePackPlan& plan,
			const std::filesystem::path& outputDir) = 0;
	};
}
