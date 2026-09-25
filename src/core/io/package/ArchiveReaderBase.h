#pragma once

#include <map>
#include <array>
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
#include "core/enums/eDataDatType.h"
#include "core/serialize/Serializer.h"
#include "core/enums/ePackageDatType.h"
#include "core/io/package/PackageEntry.h"
#include "core/io/storage/ReadOnlyFile.h"

namespace zzz::core
{
	template <typename TType>
	struct ArchiveTraitsBase
	{
		static constexpr std::size_t c_TypeCount =
			static_cast<std::size_t>(std::to_underlying(TType::Last) - std::to_underlying(TType::First) + 1);

		[[nodiscard]] static constexpr bool IsTypeAllowed(TType type) noexcept
		{
			return std::to_underlying(type) >= std::to_underlying(TType::First)
				&& std::to_underlying(type) <= std::to_underlying(TType::Last);
		}

		[[nodiscard]] static constexpr std::size_t TypeToIndex(TType type) noexcept
		{
			return static_cast<std::size_t>(std::to_underlying(type) - std::to_underlying(TType::First));
		}

		[[nodiscard]] static constexpr TType IndexToType(std::size_t index) noexcept
		{
			return static_cast<TType>(index + std::to_underlying(TType::First));
		}
	};

	template <typename TType>
	struct ArchiveTraits;

	template <>
	struct ArchiveTraits<ePackageDatType> : ArchiveTraitsBase<ePackageDatType>
	{
		static constexpr DatFileFormat c_ExpectedFormat = c_PackageDatFormat;
	};

	template <>
	struct ArchiveTraits<eDataDatType> : ArchiveTraitsBase<eDataDatType>
	{
		static constexpr DatFileFormat c_ExpectedFormat = c_DataDatFormat;
	};

	/**
	 * @class ArchiveReaderBase
	 * @brief Базовый шаблонный класс ядра для монтирования, индексации и чтения бинарных архивов .dat.
	 */
	template <typename TType> requires std::is_enum_v<TType>
	class ArchiveReaderBase
	{
		using Traits = ArchiveTraits<TType>;

	public:
		ArchiveReaderBase() = delete;
		explicit ArchiveReaderBase(
			const std::filesystem::path& path,
			NativeAppData* nativeData = nullptr)
			: m_ReadOnlyFile(path, nativeData),
			  m_ArchiveName(path.filename().string())
		{
			InitializeArchive();
		}

		virtual ~ArchiveReaderBase() = default;

		[[nodiscard]] std::string_view GetArchiveName() const noexcept { return m_ArchiveName; }

		template <TType TypeVal>
		[[nodiscard]] const std::unordered_map<Guid, PackageEntry>& GetEntries() const noexcept
		{
			static_assert(Traits::IsTypeAllowed(TypeVal), "TypeVal is not allowed for this archive");
			constexpr auto index = Traits::TypeToIndex(TypeVal);
			return m_Tables[index];
		}

		template <TType TypeVal>
		[[nodiscard]] const PackageEntry* GetEntry(const Guid& guid) const noexcept
		{
			static_assert(Traits::IsTypeAllowed(TypeVal), "TypeVal is not allowed for this archive");
			constexpr auto index = Traits::TypeToIndex(TypeVal);
			const auto& table = m_Tables[index];
			auto it = table.find(guid);
			return it != table.end() ? &it->second : nullptr;
		}

		[[nodiscard]] const PackageEntry* GetEntry(TType type, const Guid& guid) const noexcept
		{
			if (!Traits::IsTypeAllowed(type))
				return nullptr;

			const auto index = Traits::TypeToIndex(type);
			const auto& table = m_Tables[index];
			auto it = table.find(guid);
			return it != table.end() ? &it->second : nullptr;
		}

		[[nodiscard]] bool HasEntry(TType type, const Guid& guid) const noexcept
		{
			return GetEntry(type, guid) != nullptr;
		}

		[[nodiscard]] std::expected<std::span<const std::byte>, std::string> ReadRawPayload(const PackageEntry& entry) const
		{
			const auto offset = NarrowTo<std::size_t>(entry.GetOffset());
			const auto size = NarrowTo<std::size_t>(entry.GetSize());
			if (!offset || !size)
				return UNEXPECTED("Диапазон ресурса с GUID '{}' не представим адресным размером платформы", entry.GetGuid().ToString());

			if (!IsRangeInside(*offset, *size, m_ArchiveSize))
				return UNEXPECTED("Диапазон ресурса с GUID '{}' выходит за границы архива", entry.GetGuid().ToString());

			return m_ReadOnlyFile.Read(*offset, *size);
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

		std::array<std::unordered_map<Guid, PackageEntry>, Traits::c_TypeCount> m_Tables;

	private:
		std::string		m_ArchiveName;
		ReadOnlyFile	m_ReadOnlyFile;
		DatFileHeader	m_Header;
		std::size_t		m_ArchiveSize;

		void InitializeArchive()
		{
			auto archiveBytesRes = m_ReadOnlyFile.Read();
			if (!archiveBytesRes)
				THROW_RUNTIME("Не удалось прочитать архив '{}': {}", m_ArchiveName, archiveBytesRes.error());

			const auto archiveBytes = *archiveBytesRes;
			m_ArchiveSize = archiveBytes.size();
			m_Header = DatFileHeader{ Traits::c_ExpectedFormat };

			const auto headerBytes = archiveBytes.first((std::min)(archiveBytes.size(), static_cast<std::size_t>(m_Header.c_BaseHeaderSize)));

			if (auto res = m_Header.DeserializeAndValidate(headerBytes, m_ArchiveSize); !res)
				THROW_RUNTIME("Ошибка заголовка архива '{}': {}", m_ArchiveName, res.error());

			const std::size_t headerSize = m_Header.GetHeaderSize();
			for (auto& table : m_Tables)
				table.clear();

			const zU32 entryCount = m_Header.GetEntryCount();
			if (entryCount > 0)
			{
				const auto tableSize = PackageEntry::CalculateTableSize(entryCount);
				if (!tableSize)
					THROW_RUNTIME("Размер таблицы записей архива '{}' переполняет std::size_t (записей: {})", m_ArchiveName, entryCount);

				const std::size_t payloadBegin = headerSize + *tableSize;
				if (!IsRangeInside(headerSize, *tableSize, m_ArchiveSize))
					THROW_RUNTIME("Таблица записей архива '{}' выходит за границы файла: {} > {}", m_ArchiveName, payloadBegin, m_ArchiveSize);

				auto tableBytesRes = m_ReadOnlyFile.Read(headerSize, *tableSize);
				if (!tableBytesRes)
					THROW_RUNTIME("Не удалось прочитать таблицу записей архива '{}': {}", m_ArchiveName, tableBytesRes.error());

				const auto tableBytes = *tableBytesRes;

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
					if (!Traits::IsTypeAllowed(typedType))
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

					const auto typeIndex = Traits::TypeToIndex(typedType);
					m_Tables[typeIndex].emplace(entry.GetGuid(), entry);
				}
			}

			LogArchiveSummary();
		}
		void LogArchiveSummary() const
		{
#if Z_ADD_LOGGER
			std::size_t totalEntries = 0;
			for (const auto& table : m_Tables)
				totalEntries += table.size();

			DOut("========== Package Archive: {} (Total entries: {}) ==========",
				m_ArchiveName,
				totalEntries);

			m_Header.LogFileBlock("  ");

			for (std::size_t i = 0; i < Traits::c_TypeCount; ++i)
			{
				if (!m_Tables[i].empty())
				{
					DOut("  [Type] {:<16} ({})", ToString(Traits::IndexToType(i)), m_Tables[i].size());
				}
			}
#endif
		}
	};
}
