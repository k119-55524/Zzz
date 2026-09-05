#include "ArchiveWriter.h"
#include <fstream>
#include <core/io/package/PackageHeader.h>
#include <core/io/package/PackageEntry.h>

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

		// Пасс 1: Измеряем размер сериализованного заголовка + записей
		std::vector<zzz::core::PackageEntry> dummyEntries;
		dummyEntries.reserve(items.size());

		for (const auto& item : items)
		{
			dummyEntries.emplace_back(
				item.name,
				item.guid,
				item.assetType,
				0,
				item.payload.size()
			);
		}

		zzz::core::PackageHeader dummyHeader(
			magic,
			version,
			static_cast<uint32_t>(dummyEntries.size())
		);

		std::vector<std::byte> headerBuffer;
		if (!serializer.Serialize(headerBuffer, dummyHeader))
		{
			return false;
		}

		for (const auto& entry : dummyEntries)
		{
			if (!serializer.Serialize(headerBuffer, entry))
			{
				return false;
			}
		}

		const uint64_t initialOffset = headerBuffer.size();

		// Пасс 2: Строим итоговые записи с правильными смещениями
		std::vector<zzz::core::PackageEntry> finalEntries;
		finalEntries.reserve(items.size());
		uint64_t currentOffset = initialOffset;

		for (const auto& item : items)
		{
			finalEntries.emplace_back(
				item.name,
				item.guid,
				item.assetType,
				currentOffset,
				item.payload.size()
			);

			currentOffset += item.payload.size();
		}

		// Записываем финальный заголовок
		headerBuffer.clear();
		if (!serializer.Serialize(headerBuffer, dummyHeader))
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
