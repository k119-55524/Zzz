#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <expected>
#include <algorithm>
#include <filesystem>
#include <type_traits>
#include <unordered_map>

#include "core/utils/Guid.h"
#include "core/utils/Ensure.h"
#include "core/io/storage/FileSystem.h"
#include "core/utils/SafeMath.h"
#include "core/io/DatFileHeader.h"
#include "core/io/storage/ReadOnlyFile.h"
#include "core/serialize/Serializer.h"
#include "core/io/package/PackageEntry.h"

namespace zzz::core
{
	template <typename TType>
	using TypeValidator = bool(*)(TType type) noexcept;

	/**
	 * @brief Payload записи архива: borrowed span из ReadOnlyFile либо owned buffer.
	 */
	class ArchivePayload final
	{
		struct MappedView
		{
			std::shared_ptr<const ReadOnlyFile> owner;
			std::span<const std::byte> bytes;
		};

	public:
		ArchivePayload(std::shared_ptr<const ReadOnlyFile> readFile, std::span<const std::byte> bytes) noexcept
			: m_Data(MappedView{ std::move(readFile), bytes })
		{
		}

		explicit ArchivePayload(std::vector<std::byte> bytes) noexcept
			: m_Data(std::move(bytes))
		{
		}

		[[nodiscard]] std::span<const std::byte> GetSpan() const noexcept
		{
			if (const auto* mapped = std::get_if<MappedView>(&m_Data))
				return mapped->bytes;

			const auto& owned = std::get<std::vector<std::byte>>(m_Data);
			return owned;
		}

	private:
		std::variant<MappedView, std::vector<std::byte>> m_Data;
	};

	/**
	 * @struct ArchiveInitParams
	 * @brief Параметры монтирования и валидации архива пакетов .dat.
	 */
	template <typename TType>
	struct ArchiveInitParams
	{
		std::filesystem::path relativePath;
		DatFileFormat expectedFormat;
		TypeValidator<TType> isTypeAllowed = nullptr;
	};

	/**
	 * @class PackageArchive
	 * @brief Базовый шаблонный класс ядра для монтирования, индексации и чтения бинарных архивов .dat.
	 */
	template <typename TType> requires std::is_enum_v<TType>
	class PackageArchive
	{
	public:
		PackageArchive() = delete;
		explicit PackageArchive(std::shared_ptr<FileSystem> fileSystem)
			: m_FileSystem(std::move(fileSystem))
		{
			ensure(m_FileSystem != nullptr, "FileSystem не должен быть null в PackageArchive.");
		}

		virtual ~PackageArchive() = default;

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

		[[nodiscard]] std::expected<ArchivePayload, std::string> ReadRawPayload(const PackageEntry& entry) const
		{
			if (!m_FileSystem)
				return UNEXPECTED("FileSystem не инициализирован в PackageArchive.");

			const auto offset = NarrowTo<std::size_t>(entry.GetOffset());
			const auto size = NarrowTo<std::size_t>(entry.GetSize());
			if (!offset || !size)
				return UNEXPECTED("Диапазон ресурса с GUID '{}' не представим адресным размером платформы", entry.GetGuid().ToString());

			if (!IsRangeInside(*offset, *size, m_ArchiveSize))
				return UNEXPECTED("Диапазон ресурса с GUID '{}' выходит за границы архива", entry.GetGuid().ToString());

			if (m_ReadOnlyFile && m_ReadOnlyFile->IsValid())
				return ArchivePayload(m_ReadOnlyFile, m_ReadOnlyFile->Subspan(*offset, *size));

			return UNEXPECTED("ReadOnlyFile не инициализирован для архива '{}'", m_ArchivePath.string());
		}

	protected:
		void InitializeArchive(const ArchiveInitParams<TType>& params)
		{
			ensure(m_FileSystem != nullptr, "FileSystem не инициализирован в PackageArchive.");
			m_ArchivePath = params.relativePath;
			m_Header = DatFileHeader{ params.expectedFormat };

			const std::string pathStr = m_ArchivePath.generic_string();

			std::vector<std::byte> headerStorage;
			std::span<const std::byte> headerBytes;

			if constexpr (ReadOnlyFile::c_IsSupported)
			{
				auto mappedRes = ReadOnlyFile::Open(*m_FileSystem, eFileLocation::App, m_ArchivePath);
				if (!mappedRes)
					THROW_RUNTIME("Ошибка отображения архива '{}': {}", pathStr, mappedRes.error());

				m_ReadOnlyFile = std::make_shared<ReadOnlyFile>(std::move(*mappedRes));
				m_ArchiveSize = m_ReadOnlyFile->GetSize();
				const auto archiveBytes = m_ReadOnlyFile->GetSpan();
				headerBytes = archiveBytes.first((std::min)(archiveBytes.size(), static_cast<std::size_t>(m_Header.c_BaseHeaderSize)));
			}
			else
			{
				auto fileSizeRes = m_FileSystem->GetFileSize(eFileLocation::App, m_ArchivePath);
				if (!fileSizeRes)
					THROW_RUNTIME("Ошибка получения размера архива '{}': {}", pathStr, fileSizeRes.error());

				const auto archiveSize = NarrowTo<std::size_t>(*fileSizeRes);
				if (!archiveSize)
					THROW_RUNTIME("Размер архива '{}' не представим адресным размером платформы", pathStr);
				m_ArchiveSize = *archiveSize;
			}

			if (auto res = m_Header.DeserializeAndValidate(headerBytes, m_ArchiveSize); !res)
				THROW_RUNTIME("Ошибка заголовка архива '{}': {}", pathStr, res.error());

			const std::size_t headerSize = m_Header.GetHeaderSize();
			m_EntriesByGuid.clear();
			const zU32 entryCount = m_Header.GetEntryCount();
			if (entryCount > 0)
			{
				const auto tableSize = PackageEntry::CalculateTableSize(entryCount);
				if (!tableSize)
					THROW_RUNTIME("Размер таблицы записей архива '{}' переполняет std::size_t (записей: {})", pathStr, entryCount);

				const std::size_t payloadBegin = headerSize + *tableSize;
				if (!IsRangeInside(headerSize, *tableSize, m_ArchiveSize))
					THROW_RUNTIME("Таблица записей архива '{}' выходит за границы файла: {} > {}", pathStr, payloadBegin, m_ArchiveSize);

				std::span<const std::byte> tableBytes;
				if (m_ReadOnlyFile && m_ReadOnlyFile->IsValid())
				{
					tableBytes = m_ReadOnlyFile->Subspan(headerSize, *tableSize);
				}

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
						THROW_RUNTIME("Ошибка десериализации записи #{} архива '{}': {}", i, pathStr, res.error());

					const auto rawType = entry.GetAssetType();
					const auto typedType = static_cast<TType>(rawType);
					if (params.isTypeAllowed && !params.isTypeAllowed(typedType))
						THROW_RUNTIME("Запись #{} архива '{}' содержит недопустимый тип ресурса: {}", i, pathStr, rawType);

					if (!entry.IsRangeValid(payloadBegin, payloadSize))
						THROW_RUNTIME("Запись #{} архива '{}' содержит недопустимый диапазон (offset={}, size={}) при границах данных [{}, {})",
							i, pathStr, entry.GetOffset(), entry.GetSize(), payloadBegin, m_ArchiveSize);

#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
					ensure(entry.GetGuid().IsValid(), "Ресурс в пакете '{}' имеет невалидный (нулевой) GUID!", pathStr);
					if (auto it = seenGuids.find(entry.GetGuid()); it != seenGuids.end())
					{
						ensure(false,
							"Обнаружен дубликат GUID {} в архиве '{}' (конфликт типов: существующий={}, новый={})!",
							entry.GetGuid().ToString(),
							pathStr,
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
				m_ArchivePath.filename().string(),
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
			auto res = serializer.Deserialize(payloadRes->GetSpan(), offset, data);
			if (!res)
				return UNEXPECTED("Ошибка десериализации данных пакета с GUID '{}': {}.", entry.GetGuid().ToString(), res.error());

			return data;
		}

		std::shared_ptr<FileSystem> m_FileSystem;
		std::filesystem::path m_ArchivePath;

		DatFileHeader m_Header;
		std::map<TType, std::unordered_map<Guid, PackageEntry>> m_EntriesByGuid;
		std::shared_ptr<const ReadOnlyFile> m_ReadOnlyFile;
		std::size_t m_ArchiveSize = 0;
	};
}
