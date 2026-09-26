#pragma once

#include <filesystem>
#include <vector>
#include <span>
#include "IPakBuilder.h"
#include "core/io/package/PackageEntry.h"
#include "core/io/DatFileHeader.h"

namespace zzz::builder
{
	/**
	 * @class InlineDataDatBuilder
	 * @brief Сборщик бинарного архива data.dat с трёхсекционным TOC.
	 *
	 * Секции TOC:
	 * - Таблица 1: компактный массив типов внешних паков (eDataDatType);
	 * - Таблица 2: массив встроенных записей PackageEntry (Mesh, Material, Shader, Animation);
	 * - Таблица 3: массив внешних записей PackageEntry (Texture2D, AudioClip и др.);
	 * Полезная нагрузка выравнивается по 16 байтам.
	 */
	class InlineDataDatBuilder final : public IPakBuilder
	{
	public:
		InlineDataDatBuilder() = default;
		explicit InlineDataDatBuilder(std::span<const core::PackageEntry> externalEntries)
			: m_ExternalEntries(externalEntries)
		{
		}

		[[nodiscard]] PakBuildResult Build(
			const StagePackPlan& plan,
			const std::filesystem::path& outputDir) override
		{
			return BuildDataDat(plan, m_ExternalEntries, outputDir);
		}

		[[nodiscard]] PakBuildResult BuildDataDat(
			const StagePackPlan& plan,
			std::span<const core::PackageEntry> externalEntries,
			const std::filesystem::path& outputDir);

	private:
		std::span<const core::PackageEntry> m_ExternalEntries{};
	};
}
