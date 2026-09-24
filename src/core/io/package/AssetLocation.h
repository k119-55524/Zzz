#pragma once

#include <cstddef>
#include <filesystem>

#include "core/enums/eFileLocation.h"
#include "core/io/package/PackageEntry.h"

namespace zzz::core
{
	/**
	 * @struct AssetLocation
	 * @brief Физические координаты размещения ассета на диске для чтения.
	 */
	struct AssetLocation
	{
		eFileLocation location{ eFileLocation::App };
		std::filesystem::path relativePath;
		std::size_t offset = 0;
		std::size_t size = 0;
		const PackageEntry* entry = nullptr;
	};
}
