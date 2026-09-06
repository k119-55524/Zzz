#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <filesystem>
#include <core/Core.h>
#include <core/utils/Guid.h>
#include <core/utils/Version.h>
#include <core/io/DatFileHeader.h>
#include <core/serialize/Serializer.h>

namespace zzz::builder
{
	struct ArchiveItem
	{
		std::string name;
		zzz::core::Guid guid;
		uint32_t assetType{ 0 };
		std::vector<std::byte> payload;
	};

	bool WriteBinaryArchive(
		const std::filesystem::path& outPath,
		const zzz::core::DatFileHeader::Magic& magic,
		const zzz::core::Version& version,
		const std::vector<ArchiveItem>& items,
		const zzz::core::Serializer& serializer,
		uint64_t buildTime);
}
