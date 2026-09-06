#include "ArchiveWriter.h"
#include <fstream>
#include <core/io/package/PackageHeader.h>
#include <core/io/package/PackageEntry.h>
#include <core/constants/PackageConstants.h>
#include <core/utils/ThrowWrappers.h>

namespace zzz::builder
{
	bool WriteBinaryArchive(
		const std::filesystem::path& outPath,
		const zzz::core::FileHeader<3>& magic,
		const zzz::core::Version& version,
		const std::vector<ArchiveItem>& items,
		const zzz::core::Serializer& serializer)
	{
		std::filesystem::create_directories(outPath.parent_path());
		std::ofstream outFile(outPath, std::ios::binary);
		if (!outFile.is_open())
		{
			return false;
		}

		// Сериализатор сам сформирует заголовок и таблицу и сдвинет указатель на точный размер
		zzz::core::PackageHeader header(
			magic,
			version,
			static_cast<uint32_t>(items.size())
		);

		std::vector<zzz::core::PackageEntry> tempEntries;
		tempEntries.reserve(items.size());
		for (const auto& item : items)
		{
			auto nameRes = zzz::core::PackageEntry::NameStringType::Create(item.name);
			if (!nameRes)
			{
				DOutError("ArchiveWriter: Имя ресурса '{}' превышает лимит в {} символов: {}",
					item.name, zzz::core::c_MaxAssetNameLength, nameRes.error());
				return false;
			}

			tempEntries.emplace_back(*nameRes, item.guid, item.assetType, 0, item.payload.size());
		}

		std::vector<std::byte> headerBuffer;
		if (!serializer.Serialize(headerBuffer, header))
		{
			return false;
		}

		for (const auto& entry : tempEntries)
		{
			if (!serializer.Serialize(headerBuffer, entry))
			{
				return false;
			}
		}

		const uint64_t initialOffset = headerBuffer.size();

		std::vector<zzz::core::PackageEntry> finalEntries;
		finalEntries.reserve(items.size());
		uint64_t currentOffset = initialOffset;

		for (const auto& item : items)
		{
			auto nameRes = zzz::core::PackageEntry::NameStringType::Create(item.name);
			if (!nameRes)
			{
				DOutError("ArchiveWriter: Имя ресурса '{}' превышает лимит в {} символов: {}",
					item.name, zzz::core::c_MaxAssetNameLength, nameRes.error());
				return false;
			}

			finalEntries.emplace_back(
				*nameRes,
				item.guid,
				item.assetType,
				currentOffset,
				item.payload.size()
			);

			currentOffset += item.payload.size();
		}

		headerBuffer.clear();
		headerBuffer.reserve(static_cast<std::size_t>(initialOffset));
		if (!serializer.Serialize(headerBuffer, header))
		{
			return false;
		}

		for (const auto& entry : finalEntries)
		{
			if (!serializer.Serialize(headerBuffer, entry))
			{
				return false;
			}
		}

		// Записываем заголовок и таблицу записей в файл
		outFile.write(reinterpret_cast<const char*>(headerBuffer.data()), headerBuffer.size());

		// Записываем бинарные полезные нагрузки
		for (const auto& item : items)
		{
			if (!item.payload.empty())
			{
				outFile.write(reinterpret_cast<const char*>(item.payload.data()), item.payload.size());
			}
		}

		outFile.close();
		return true;
	}
}
