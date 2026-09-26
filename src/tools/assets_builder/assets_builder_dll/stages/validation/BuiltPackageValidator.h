#pragma once

#include <filesystem>
#include <string>
#include <expected>
#include <vector>
#include "core/io/package/PackageEntry.h"
#include "core/enums/eDataDatType.h"

namespace zzz::builder
{
	struct PackageValidationReport
	{
		bool                                 isValid{ false };
		std::string                          errorMessage;
		uint64_t                             packageDatTimestamp{ 0 };
		uint64_t                             dataDatTimestamp{ 0 };
		uint32_t                             inlineCount{ 0 };
		uint32_t                             externalCount{ 0 };
		std::vector<core::eDataDatType>      packManifestTypes;
		std::vector<core::PackageEntry>      inlineEntries;
		std::vector<core::PackageEntry>      externalEntries;
	};

	/**
	 * @class BuiltPackageValidator
	 * @brief Независимый парсер и валидатор готового комплекта пакетов assets.new/ перед публикацией.
	 *
	 * С диска заново читает package.dat, data.dat и все внешние пакеты N.dat, проверяет сигнатуры,
	 * версии, единый timestamp, инварианты смещений, кратность 16, границы файлов и отсутствие коллизий.
	 */
	class BuiltPackageValidator
	{
	public:
		[[nodiscard]] static PackageValidationReport Validate(const std::filesystem::path& assetsDir);
	};
}
