#pragma once

#include <string>
#include <expected>
#include <vector>
#include <cstddef>
#include <filesystem>
#include <core/Core.h>
#include <core/utils/Guid.h>
#include <core/utils/Version.h>
#include <core/io/DatFileHeader.h>
#include <core/serialize/Serializer.h>
#include <core/enums/eFileLocation.h>

namespace zzz::builder
{
	struct ArchiveItem
	{
		std::string name;
		zzz::core::Guid guid;
		uint32_t assetType{ 0 };
		std::vector<std::byte> payload;
	};

	[[nodiscard]] std::expected<void, std::string> WriteBinaryArchiveImpl(
		const std::filesystem::path& outPath,
		const zzz::core::ISerializable& header,
		const std::vector<ArchiveItem>& items,
		const zzz::core::Serializer& serializer);

	[[nodiscard]] inline std::expected<void, std::string> WriteBinaryArchive(
		const std::filesystem::path& outPath,
		const zzz::core::DatFileFormat& format,
		const std::vector<ArchiveItem>& items,
		const zzz::core::Serializer& serializer,
		uint64_t buildTime)
	{
		const zzz::core::DatFileHeader header(
			format,
			static_cast<uint32_t>(items.size()),
			buildTime
		);
		return WriteBinaryArchiveImpl(outPath, header, items, serializer);
	}
}
