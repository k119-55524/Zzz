#pragma once

#include <map>
#include <string>
#include <expected>
#include <algorithm>
#include <filesystem>
#include <type_traits>
#include <unordered_map>

#include "core/utils/Guid.h"
#include "core/utils/Ensure.h"
#include "core/utils/SafeMath.h"
#include "core/io/DatFileHeader.h"
#include "core/serialize/Serializer.h"
#include "core/io/package/PackageEntry.h"
#include "core/io/storage/ReadOnlyFile.h"

namespace zzz::core
{
	template <typename TType>
	using TypeValidator = bool(*)(TType type) noexcept;

	/**
	 * @class ArchiveReaderBase
	 * @brief Базовый шаблонный класс ядра для монтирования, индексации и чтения бинарных архивов .dat.
	 */
	template <typename TType> requires std::is_enum_v<TType>
	class ArchiveReaderBase
	{
	public:
		ArchiveReaderBase() = delete;
		explicit ArchiveReaderBase(
			const std::filesystem::path& path,
			DatFileFormat expectedFormat,
			TypeValidator<TType> isTypeAllowed)
			: m_ReadOnlyFile(path),
			  m_ArchiveName(path.filename().string())
		{
			ensure(isTypeAllowed != nullptr, "isTypeAllowed не должен быть null в ArchiveReaderBase.");
			InitializeArchive(expectedFormat, isTypeAllowed);
		}

		virtual ~ArchiveReaderBase() = default;

		[[nodiscard]] const std::filesystem::path& GetPath() const noexcept { return m_ReadOnlyFile.GetPath(); }
		[[nodiscard]] std::string_view GetArchiveName() const noexcept { return m_ArchiveName; }

		[[nodiscard]] const PackageEntry* GetEntry(TType type, const Guid& guid) const noexcept
		{
			auto typeIt = m_EntriesByGuid.find(type);
			if (typeIt == m_EntriesByGuid.end())
				return nullptr;

			auto guidIt = typeIt->second.find(guid);
			if (guidIt == typeIt->second.end())
				return nullptr;

			return &guidIt->second;
		}

		[[nodiscard]] std::expected<std::span<const std::byte>, std::string> ReadRawPayload(const PackageEntry& entry) const
		{
			if (!m_ReadOnlyFile.IsValid())
				return UNEXPECTED("ReadOnlyFile не инициализирован для архива '{}'", m_ArchiveName);

			const auto offset = NarrowTo<std::size_t>(entry.GetOffset());
			const auto size = NarrowTo<std::size_t>(entry.GetSize());
			if (!offset || !size)
				return UNEXPECTED("Диапазон ресурса с GUID '{}' не представим адресным размером платформы", entry.GetGuid().ToString());

			if (!IsRangeInside(*offset, *size, m_ArchiveSize))
				return UNEXPECTED("Диапазон ресурса с GUID '{}' выходит за границы архива", entry.GetGuid().ToString());

			return m_ReadOnlyFile.Subspan(*offset, *size);
		}

	private:
		void InitializeArchive(DatFileFormat expectedFormat, TypeValidator<TType> isTypeAllowed)
		{
			if (!m_ReadOnlyFile.IsValid())
				THROW_RUNTIME("Ошибка архива '{}': файл не открыт или повреждён ({}).", m_ArchiveName, m_ReadOnlyFile.GetError());

			m_ArchiveSize = m_ReadOnlyFile.GetSize();
			m_Header = DatFileHeader{ expectedFormat };

			const auto archiveBytes = m_ReadOnlyFile.GetSpan();
			const auto headerBytes = archiveBytes.first((std::min)(archiveBytes.size(), static_cast<std::size_t>(m_Header.c_BaseHeaderSize)));

			if (auto res = m_Header.DeserializeAndValidate(headerBytes, m_ArchiveSize); !res)
				THROW_RUNTIME("Ошибка заголовка архива '{}': {}", m_ArchiveName, res.error());

			const std::size_t headerSize = m_Header.GetHeaderSize();
			m_EntriesByGuid.clear();
			const zU32 entryCount = m_Header.GetEntryCount();
			if (entryCount > 0)
			{
				const auto tableSize = PackageEntry::CalculateTableSize(entryCount);
				if (!tableSize)
					THROW_RUNTIME("Размер таблицы записей архива '{}' переполняет std::size_t (записей: {})", m_ArchiveName, entryCount);

				const std::size_t payloadBegin = headerSize + *tableSize;
				if (!IsRangeInside(headerSize, *tableSize, m_ArchiveSize))
					THROW_RUNTIME("Таблица записей архива '{}' выходит за границы файла: {} > {}", m_ArchiveName, payloadBegin, m_ArchiveSize);

				const auto tableBytes = m_ReadOnlyFile.Subspan(headerSize, *tableSize);

#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
				std::unordered_map<Guid, zU32> seenGuids;
				seenGuids.reserve(entryCount);
#endif

				const std::size_t payloadSize = m_ArchiveSize - payloadBegin;
				std::size_t tableOffset = 0;
				Serializer serializer;

				for (zU32 i = 0; i < entryCount; ++i)
				{
					PackageEntry entry{};
					if (auto res = serializer.Deserialize(tableBytes, tableOffset, entry); !res)
						THROW_RUNTIME("Ошибка десериализации записи #{} архива '{}': {}", i, m_ArchiveName, res.error());

					const auto rawType = entry.GetAssetType();
					const auto typedType = static_cast<TType>(rawType);
					if (!isTypeAllowed(typedType))
						THROW_RUNTIME("Запись #{} архива '{}' содержит недопустимый тип ресурса: {}", i, m_ArchiveName, rawType);

					if (!entry.IsRangeValid(payloadBegin, payloadSize))
						THROW_RUNTIME("Запись #{} архива '{}' содержит недопустимый диапазон (offset={}, size={}) при границах данных [{}, {})",
							i, m_ArchiveName, entry.GetOffset(), entry.GetSize(), payloadBegin, m_ArchiveSize);

#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
					ensure(entry.GetGuid().IsValid(), "Ресурс в пакете '{}' имеет невалидный (нулевой) GUID!", m_ArchiveName);
					if (auto it = seenGuids.find(entry.GetGuid()); it != seenGuids.end())
					{
						ensure(false,
							"Обнаружен дубликат GUID {} в архиве '{}' (конфликт типов: существующий={}, новый={})!",
							entry.GetGuid().ToString(),
							m_ArchiveName,
							it->second,
							rawType);
					}
					seenGuids.emplace(entry.GetGuid(), rawType);
#endif

					m_EntriesByGuid[typedType].emplace(entry.GetGuid(), entry);
				}
			}

			LogArchiveSummary();
		}

		void LogArchiveSummary() const
		{
#if Z_ADD_LOGGER
			std::size_t totalEntries = 0;
			for (const auto& [type, entries] : m_EntriesByGuid)
				totalEntries += entries.size();

			DOut("========== Package Archive: {} (Total entries: {}) ==========",
				m_ArchiveName,
				totalEntries);

			m_Header.LogFileBlock("  ");

			for (const auto& [type, entries] : m_EntriesByGuid)
			{
				DOut("  [Type] {:<16} ({})", ToString(type), entries.size());

				for (const auto& [guid, entry] : entries)
				{
					entry.LogFileBlock("    ");
					LogEntryDetails(entry);
				}
				DOut("");
			}
#endif // Z_ADD_LOGGER
		}

	protected:
		virtual void LogEntryDetails([[maybe_unused]] const PackageEntry& entry) const {}

		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::expected<T, std::string> DeserializeEntryRaw(const PackageEntry& entry) const
		{
			auto payloadRes = ReadRawPayload(entry);
			if (!payloadRes)
				return UNEXPECTED("{}", payloadRes.error());

			std::size_t offset = 0;
			Serializer serializer;
			T data{};
			auto res = serializer.Deserialize(*payloadRes, offset, data);
			if (!res)
				return UNEXPECTED("Ошибка десериализации данных пакета с GUID '{}': {}.", entry.GetGuid().ToString(), res.error());

			return data;
		}

		std::map<TType, std::unordered_map<Guid, PackageEntry>> m_EntriesByGuid;

	private:
		std::string		m_ArchiveName;
		ReadOnlyFile	m_ReadOnlyFile;
		DatFileHeader	m_Header;
		std::size_t		m_ArchiveSize;
	};
}
