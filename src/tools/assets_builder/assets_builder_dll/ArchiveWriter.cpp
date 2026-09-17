#include "ArchiveWriter.h"
#include <fstream>
#include <format>
#include <core/io/DatFileHeader.h>
#include <core/io/package/PackageEntry.h>
#include <core/constants/PackageConstants.h>
#include <core/utils/ThrowWrappers.h>

namespace zzz::builder
{
	std::expected<void, std::string> WriteBinaryArchive(
		const std::filesystem::path& outPath,
		const zzz::core::DatFileHeader::Magic& magic,
		const zzz::core::Version& version,
		const std::vector<ArchiveItem>& items,
		const zzz::core::Serializer& serializer,
		uint64_t buildTime)
	{
		std::error_code directoryError;
		std::filesystem::create_directories(outPath.parent_path(), directoryError);
		if (directoryError)
		{
			return std::unexpected(std::format(
				"Не удалось создать каталог архива '{}': {}", outPath.parent_path().string(), directoryError.message()));
		}

		std::ofstream outFile(outPath, std::ios::binary);
		if (!outFile.is_open())
		{
			return std::unexpected("Не удалось открыть архив для записи: " + outPath.string());
		}

		// Сериализатор сам сформирует заголовок и таблицу и сдвинет указатель на точный размер
		zzz::core::DatFileHeader header(
			magic,
			version,
			static_cast<uint32_t>(items.size()),
			buildTime
		);

		std::vector<zzz::core::PackageEntry> tempEntries;
		tempEntries.reserve(items.size());
		for (const auto& item : items)
		{
			auto nameRes = zzz::core::PackageEntry::NameStringType::Create(item.name);
			if (!nameRes)
			{
				return std::unexpected(std::format(
					"Имя ресурса '{}' превышает лимит в {} символов: {}",
					item.name, zzz::core::c_MaxAssetNameLength, nameRes.error()));
			}

			tempEntries.emplace_back(*nameRes, item.guid, item.assetType, 0, item.payload.size());
		}

		std::vector<std::byte> headerBuffer;
		if (auto res = serializer.Serialize(headerBuffer, header); !res)
		{
			return std::unexpected("Ошибка сериализации заголовка архива '" + outPath.string() + "': " + res.error());
		}

		for (const auto& entry : tempEntries)
		{
			if (auto res = serializer.Serialize(headerBuffer, entry); !res)
			{
				return std::unexpected("Ошибка сериализации таблицы архива '" + outPath.string() + "': " + res.error());
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
				return std::unexpected(std::format(
					"Имя ресурса '{}' превышает лимит в {} символов: {}",
					item.name, zzz::core::c_MaxAssetNameLength, nameRes.error()));
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
		if (auto res = serializer.Serialize(headerBuffer, header); !res)
		{
			return std::unexpected("Ошибка финальной сериализации заголовка архива '" + outPath.string() + "': " + res.error());
		}

		for (const auto& entry : finalEntries)
		{
			if (auto res = serializer.Serialize(headerBuffer, entry); !res)
			{
				return std::unexpected("Ошибка финальной сериализации таблицы архива '" + outPath.string() + "': " + res.error());
			}
		}

		// Записываем заголовок и таблицу записей в файл
		outFile.write(reinterpret_cast<const char*>(headerBuffer.data()), headerBuffer.size());
		if (!outFile)
			return std::unexpected("Ошибка записи заголовка архива: " + outPath.string());

		// Записываем бинарные полезные нагрузки
		for (const auto& item : items)
		{
			if (!item.payload.empty())
			{
				outFile.write(reinterpret_cast<const char*>(item.payload.data()), item.payload.size());
				if (!outFile)
					return std::unexpected("Ошибка записи ресурса '" + item.name + "' в архив '" + outPath.string() + "'.");
			}
		}

		outFile.close();
		if (outFile.fail())
			return std::unexpected("Ошибка завершения записи архива: " + outPath.string());

		return {};
	}
}
