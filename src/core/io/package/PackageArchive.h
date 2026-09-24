#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <expected>
#include <filesystem>
#include <type_traits>
#include <unordered_map>

#include "core/utils/Guid.h"
#include "core/utils/Ensure.h"
#include "core/io/FileSystem.h"
#include "core/utils/SafeMath.h"
#include "core/io/DatFileHeader.h"
#include "core/serialize/Serializer.h"
#include "core/io/package/PackageEntry.h"
#include "core/io/package/AssetLocation.h"

namespace zzz::core
{
	template <typename TType>
	using TypeValidator = bool(*)(TType type) noexcept;

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

		[[nodiscard]] std::expected<AssetLocation, std::string> GetAssetLocation(const PackageEntry& entry) const
		{
			const auto offset = NarrowTo<std::size_t>(entry.GetOffset());
			const auto size = NarrowTo<std::size_t>(entry.GetSize());
			if (!offset || !size)
				return UNEXPECTED("Диапазон ресурса с GUID '{}' не представим адресным размером платформы", entry.GetGuid().ToString());

			return AssetLocation{
				.location = eFileLocation::App,
				.relativePath = m_ArchivePath,
				.offset = *offset,
				.size = *size,
				.entry = &entry
			};
		}

		[[nodiscard]] std::expected<AssetLocation, std::string> GetAssetLocation(TType type, const Guid& guid) const
		{
			const auto* entry = GetEntry(type, guid);
			if (!entry)
				return UNEXPECTED("Package entry of type {} with GUID '{}' was not found.", ToString(type), guid.ToString());

			return GetAssetLocation(*entry);
		}

	protected:
		void InitializeArchive(const ArchiveInitParams<TType>& params)
		{
			ensure(m_FileSystem != nullptr, "FileSystem не инициализирован в PackageArchive.");
			m_ArchivePath = params.relativePath;
			m_Header = DatFileHeader{ params.expectedFormat };

			const std::string pathStr = m_ArchivePath.generic_string();

			auto fileSizeRes = m_FileSystem->GetFileSize(eFileLocation::App, m_ArchivePath);
			if (!fileSizeRes)
				THROW_RUNTIME("Ошибка получения размера архива '{}': {}", pathStr, fileSizeRes.error());

			const std::uintmax_t fileSize = *fileSizeRes;
			auto headerBytesRes = m_FileSystem->ReadBytes(eFileLocation::App, m_ArchivePath, 0, m_Header.c_BaseHeaderSize);
			if (!headerBytesRes)
				THROW_RUNTIME("Ошибка чтения заголовка архива '{}': {}", pathStr, headerBytesRes.error());

			if (auto res = m_Header.DeserializeAndValidate(*headerBytesRes, fileSize); !res)
				THROW_RUNTIME("Ошибка заголовка архива '{}': {}", pathStr, res.error());

			const std::size_t headerSize = m_Header.GetHeaderSize();
			m_EntriesByGuid.clear();
			const zU32 entryCount = m_Header.GetEntryCount();
			if (entryCount > 0)
			{
				const auto tableSize = PackageEntry::CalculateTableSize(entryCount);
				if (!tableSize)
					THROW_RUNTIME("Размер таблицы записей архива '{}' переполняет std::size_t (записей: {})", pathStr, entryCount);

				const std::uintmax_t payloadBegin = headerSize + static_cast<std::uintmax_t>(*tableSize);
				if (!IsRangeInside<std::uintmax_t>(headerSize, *tableSize, fileSize))
					THROW_RUNTIME("Таблица записей архива '{}' выходит за границы файла: {} > {}", pathStr, payloadBegin, fileSize);

				auto tableBufferRes = m_FileSystem->ReadBytes(eFileLocation::App, m_ArchivePath, headerSize, *tableSize);
				if (!tableBufferRes)
					THROW_RUNTIME("Ошибка чтения таблицы записей архива '{}': {}", pathStr, tableBufferRes.error());

#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
				std::unordered_map<Guid, zU32> seenGuids;
				seenGuids.reserve(entryCount);
#endif

				const std::uintmax_t payloadSize = fileSize - payloadBegin;
				std::size_t tableOffset = 0;
				Serializer serializer;

				for (zU32 i = 0; i < entryCount; ++i)
				{
					PackageEntry entry{};
					if (auto res = serializer.Deserialize(*tableBufferRes, tableOffset, entry); !res)
						THROW_RUNTIME("Ошибка десериализации записи #{} архива '{}': {}", i, pathStr, res.error());

					const auto rawType = entry.GetAssetType();
					const auto typedType = static_cast<TType>(rawType);
					if (params.isTypeAllowed && !params.isTypeAllowed(typedType))
						THROW_RUNTIME("Запись #{} архива '{}' содержит недопустимый тип ресурса: {}", i, pathStr, rawType);

					if (!entry.IsRangeValid(payloadBegin, payloadSize))
						THROW_RUNTIME("Запись #{} архива '{}' содержит недопустимый диапазон (offset={}, size={}) при границах данных [{}, {})",
							i, pathStr, entry.GetOffset(), entry.GetSize(), payloadBegin, fileSize);

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

		[[nodiscard]] std::expected<std::vector<std::byte>, std::string> ReadRawBytes(const PackageEntry& entry) const
		{
			if (!m_FileSystem)
				return UNEXPECTED("FileSystem не инициализирован в PackageArchive.");

			const auto offset = NarrowTo<std::size_t>(entry.GetOffset());
			const auto size = NarrowTo<std::size_t>(entry.GetSize());
			if (!offset || !size)
				return UNEXPECTED("Диапазон ресурса с GUID '{}' не представим адресным размером платформы", entry.GetGuid().ToString());

			if (*size == 0)
				return std::vector<std::byte>{};

			return m_FileSystem->ReadBytes(eFileLocation::App, m_ArchivePath, *offset, *size);
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
			auto bufferRes = ReadRawBytes(entry);
			if (!bufferRes)
				return UNEXPECTED("{}", bufferRes.error());

			std::size_t offset = 0;
			Serializer serializer;
			T data{};
			auto res = serializer.Deserialize(*bufferRes, offset, data);
			if (!res)
				return UNEXPECTED("Ошибка десериализации данных пакета с GUID '{}': {}.", entry.GetGuid().ToString(), res.error());

			return data;
		}

		DatFileHeader m_Header;
		std::map<TType, std::unordered_map<Guid, PackageEntry>> m_EntriesByGuid;
		std::shared_ptr<FileSystem> m_FileSystem;
		std::filesystem::path m_ArchivePath;
	};
}
