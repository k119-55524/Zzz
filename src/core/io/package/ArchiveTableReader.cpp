#include <format>
#include <unordered_set>

#include "core/utils/Ensure.h"
#include "core/logger/logger.h"
#include "core/utils/SafeRange.h"
#include "core/serialize/Serializer.h"

#include "ArchiveTableReader.h"

Z_SET_LOG_CATEGORY(::zzz::core::Assets);

namespace zzz::core
{
	[[nodiscard]] std::expected<ArchiveTable, std::string> ReadArchiveTable(
		const FileSystem& fileSystem,
		eFileLocation location,
		const std::filesystem::path& relativePath,
		const DatFileHeader::Magic& expectedMagic,
		zU8 expectedMajorVersion,
		ArchiveEntryTypeValidator isValidType)
	{
		ensure(isValidType != nullptr, "ReadArchiveTable: не задана проверка типа записи.");

		const std::string pathStr = relativePath.generic_string();

		// 1. Размер файла.
		auto fileSizeRes = fileSystem.GetFileSize(location, relativePath);
		if (!fileSizeRes)
			return UNEXPECTED("Не удалось получить размер архива '{}': {}", pathStr, fileSizeRes.error());

		const std::uintmax_t fileSize = *fileSizeRes;
		if (fileSize < DatFileHeader::BinarySize())
			return UNEXPECTED("Архив '{}' меньше обязательного заголовка: {} < {}", pathStr, fileSize, DatFileHeader::BinarySize());

		// 2. Заголовок.
		auto headerBufferRes = fileSystem.ReadBytes(location, relativePath, 0, DatFileHeader::BinarySize());
		if (!headerBufferRes)
			return UNEXPECTED("Не удалось прочитать заголовок архива '{}': {}", pathStr, headerBufferRes.error());

		ArchiveTable table{};
		Serializer serializer;
		std::size_t headerOffset = 0;
		if (auto res = serializer.Deserialize(*headerBufferRes, headerOffset, table.header); !res)
			return UNEXPECTED("Ошибка десериализации заголовка архива '{}': {}", pathStr, res.error());

		if (auto res = table.header.Validate(expectedMagic, expectedMajorVersion); !res)
			return UNEXPECTED("Некорректный заголовок архива '{}': {}", pathStr, res.error());

		// 3. Таблица записей.
		const zU32 entryCount = table.header.GetEntryCount();
		const auto tableSize = CheckedMul<std::size_t>(entryCount, PackageEntry::BinarySize());
		if (!tableSize)
			return UNEXPECTED("Размер таблицы записей архива '{}' переполняет std::size_t (записей: {})", pathStr, entryCount);

		const std::uintmax_t payloadBegin = DatFileHeader::BinarySize() + static_cast<std::uintmax_t>(*tableSize);
		if (!IsRangeInside<std::uintmax_t>(DatFileHeader::BinarySize(), *tableSize, fileSize))
			return UNEXPECTED("Таблица записей архива '{}' выходит за границы файла: {} > {}", pathStr, payloadBegin, fileSize);

		if (entryCount == 0)
			return table;

		auto tableBufferRes = fileSystem.ReadBytes(location, relativePath, DatFileHeader::BinarySize(), *tableSize);
		if (!tableBufferRes)
			return UNEXPECTED("Ошибка чтения таблицы записей архива '{}': {}", pathStr, tableBufferRes.error());

		// 4. Записи.
		table.entries.reserve(entryCount);
		const std::uintmax_t payloadSize = fileSize - payloadBegin;

#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
		std::unordered_set<Guid> seenGuids;
		seenGuids.reserve(entryCount);
#endif

		std::size_t tableOffset = 0;
		for (zU32 i = 0; i < entryCount; ++i)
		{
			PackageEntry entry{};
			if (auto res = serializer.Deserialize(*tableBufferRes, tableOffset, entry); !res)
				return UNEXPECTED("Ошибка десериализации записи #{} архива '{}': {}", i, pathStr, res.error());

			if (!isValidType(entry.GetAssetType()))
				return UNEXPECTED("Запись #{} архива '{}' содержит недопустимый тип: {}", i, pathStr, entry.GetAssetType());

			const std::uintmax_t entryOffset = entry.GetOffset();
			const std::uintmax_t entrySize = entry.GetSize();
			if (entryOffset < payloadBegin || !IsRangeInside<std::uintmax_t>(entryOffset - payloadBegin, entrySize, payloadSize))
			{
				return UNEXPECTED("Запись #{} архива '{}' содержит недопустимый диапазон (offset={}, size={}) при границах данных [{}, {})",
					i, pathStr, entryOffset, entrySize, payloadBegin, fileSize);
			}

#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
			ensure(entry.GetGuid().IsValid(), "Запись #{} архива '{}' имеет невалидный (нулевой) GUID!", i, pathStr);
			ensure(seenGuids.insert(entry.GetGuid()).second, "Обнаружен дубликат GUID {} в архиве '{}'!", entry.GetGuid().ToString(), pathStr);
#endif

			table.entries.push_back(entry);
		}

		return table;
	}
}
