#pragma once

#include <filesystem>
#include <vector>
#include <span>
#include "IPakBuilder.h"
#include "core/enums/eDataDatType.h"
#include "core/io/DatFileHeader.h"

namespace zzz::builder
{
	/**
	 * @class StandardExternalPakBuilder
	 * @brief Сборщик внешнего .dat файла (0.dat, 1.dat и т.д.).
	 *
	 * Записывает 4 КБ заголовок PakFileHeader ("ZPK", v1.0.0), запекает ресурсы
	 * через соответствующие импортёры и размещает блобы с выравниванием по 16 байт.
	 */
	class StandardExternalPakBuilder final : public IPakBuilder
	{
	public:
		StandardExternalPakBuilder() = default;

		[[nodiscard]] PakBuildResult Build(
			const StagePackPlan& plan,
			const std::filesystem::path& outputDir) override;

		[[nodiscard]] PakBuildResult BuildPak(
			core::eDataDatType type,
			std::span<const ProjectAssetInfo> assets,
			const StagePackPlan& plan,
			const std::filesystem::path& outputDir);
	};
}
