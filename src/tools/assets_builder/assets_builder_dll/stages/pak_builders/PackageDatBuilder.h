#pragma once

#include <filesystem>
#include "IPakBuilder.h"
#include "../StagePackPlan.h"

namespace zzz::builder
{
	/**
	 * @class PackageDatBuilder
	 * @brief Сборщик системного архива package.dat (манифест, декларации представлений, сцены).
	 */
	class PackageDatBuilder final : public IPakBuilder
	{
	public:
		PackageDatBuilder() = default;

		[[nodiscard]] PakBuildResult Build(
			const StagePackPlan& plan,
			const std::filesystem::path& outputDir) override;
	};
}
