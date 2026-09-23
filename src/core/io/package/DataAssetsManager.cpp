
#include <type_traits>

#include "core/utils/Ensure.h"
#include "core/utils/SafeRange.h"
#include "core/io/DatFileHeader.h"
#include "core/constants/PackagesConstants.h"

#include "DataAssetsManager.h"

Z_SET_LOG_CATEGORY(::zzz::core::Assets);

namespace
{
	/// @brief Типы ресурсов, которые допустимо хранить в архиве data.dat.
	/// Манифест, окна и сцены хранятся в package.dat и в data.dat считаются повреждением.
	[[nodiscard]] constexpr bool IsDataArchiveResourceType(::zzz::core::eResourceType type) noexcept
	{
		using ::zzz::core::eResourceType;
		switch (type)
		{
		case eResourceType::Prefab:
		case eResourceType::Mesh:
		case eResourceType::Material:
		case eResourceType::Shader:
		case eResourceType::Animation:
		case eResourceType::Texture2D:
		case eResourceType::AudioClip:
		case eResourceType::Video:
		case eResourceType::Font:
		case eResourceType::BinaryData:
			return true;
		default:
			return false;
		}
	}
}

namespace zzz::core
{
	DataAssetsManager::DataAssetsManager(const FileSystem& fileSystem)
	{
		Initialize(fileSystem);
	}

	void DataAssetsManager::Initialize(const FileSystem& fileSystem)
	{
		const std::string pathStr = c_DataPackageRelativePath.generic_string();

		auto fileSizeRes = fileSystem.GetFileSize(eFileLocation::App, c_DataPackageRelativePath);
		if (!fileSizeRes)
		{
			THROW_RUNTIME("Ошибка получения размера архива данных '{}': {}", pathStr, fileSizeRes.error());
		}

		const std::uintmax_t fileSize = *fileSizeRes;

		auto headerBytesRes = fileSystem.ReadBytes(eFileLocation::App, c_DataPackageRelativePath, 0, m_Header.c_BaseHeaderSize);
		if (!headerBytesRes)
		{
			THROW_RUNTIME("Ошибка чтения заголовка архива данных '{}': {}", pathStr, headerBytesRes.error());
		}

		if (auto res = m_Header.DeserializeAndValidate(*headerBytesRes, fileSize); !res)
		{
			THROW_RUNTIME("Ошибка заголовка архива данных '{}': {}", pathStr, res.error());
		}

		const std::size_t headerSize = m_Header.GetHeaderSize();

		m_Entries.clear();

		const zU32 entryCount = m_Header.GetEntryCount();
		// Чтение оглавления архива (каталог PackageEntry: GUID, тип ресурса, его смещение и размер в файле).
		// Сами бинарные данные ассетов в память здесь не загружаются.
		if (entryCount > 0)
		{
			const auto tableSize = CheckedMul<std::size_t>(entryCount, PackageEntry::BinarySize());
			if (!tableSize)
			{
				THROW_RUNTIME("Размер таблицы записей архива '{}' переполняет std::size_t (записей: {})", pathStr, entryCount);
			}

			const std::uintmax_t payloadBegin = headerSize + static_cast<std::uintmax_t>(*tableSize);
			if (!IsRangeInside<std::uintmax_t>(headerSize, *tableSize, fileSize))
			{
				THROW_RUNTIME("Таблица записей архива '{}' выходит за границы файла: {} > {}", pathStr, payloadBegin, fileSize);
			}

			auto tableBufferRes = fileSystem.ReadBytes(eFileLocation::App, c_DataPackageRelativePath, headerSize, *tableSize);
			if (!tableBufferRes)
			{
				THROW_RUNTIME("Ошибка чтения таблицы записей архива '{}': {}", pathStr, tableBufferRes.error());
			}

			m_Entries.reserve(entryCount);
			const std::uintmax_t payloadSize = fileSize - payloadBegin;
			std::size_t tableOffset = 0;
			Serializer serializer;

			for (zU32 i = 0; i < entryCount; ++i)
			{
				PackageEntry entry{};
				if (auto res = serializer.Deserialize(*tableBufferRes, tableOffset, entry); !res)
				{
					THROW_RUNTIME("Ошибка десериализации записи #{} архива '{}': {}", i, pathStr, res.error());
				}

				const auto rawType = NarrowTo<std::underlying_type_t<eResourceType>>(entry.GetAssetType());
				if (!rawType || !IsDataArchiveResourceType(static_cast<eResourceType>(*rawType)))
				{
					THROW_RUNTIME("Запись #{} архива '{}' содержит недопустимый тип ресурса: {}", i, pathStr, entry.GetAssetType());
				}

				const std::uintmax_t entryOffset = entry.GetOffset();
				const std::uintmax_t entrySize = entry.GetSize();
				if (entryOffset < payloadBegin || !IsRangeInside<std::uintmax_t>(entryOffset - payloadBegin, entrySize, payloadSize))
				{
					THROW_RUNTIME("Запись #{} архива '{}' содержит недопустимый диапазон (offset={}, size={}) при границах данных [{}, {})",
						i, pathStr, entryOffset, entrySize, payloadBegin, fileSize);
				}

#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
				ensure(entry.GetGuid().IsValid(), "Ресурс в пакете data.dat имеет невалидный (нулевой) GUID!");
				if (auto it = m_Entries.find(entry.GetGuid()); it != m_Entries.end())
				{
					ensure(false,
						"Обнаружен дубликат GUID {} в архиве data.dat (конфликт типов: существующий={}, новый={})!",
						entry.GetGuid().ToString(),
						ToString(static_cast<eResourceType>(it->second.GetAssetType())),
						ToString(static_cast<eResourceType>(entry.GetAssetType())));
				}
#endif
				m_Entries.emplace(entry.GetGuid(), entry);
			}
		}

		LogDataEntriesSummary(m_Header);
	}

	[[nodiscard]] const PackageEntry* DataAssetsManager::GetEntryPtr(const Guid& guid) const
	{
		auto it = m_Entries.find(guid);
		if (it == m_Entries.end())
			return nullptr;

		return &it->second;
	}

	[[nodiscard]] std::expected<AssetLocation, std::string> DataAssetsManager::GetAssetLocation(eResourceType type, const Guid& guid) const
	{
		const PackageEntry* entry = GetEntryPtr(guid);
		if (!entry)
		{
			return UNEXPECTED("Ресурс с GUID '{}' не найден в оглавлении data.dat", guid.ToString());
		}

		if (entry->GetAssetType() != static_cast<zU32>(type))
		{
			return UNEXPECTED("Несоответствие типа ресурса с GUID '{}': ожидался {}, в архиве {}",
				guid.ToString(), ToString(type), ToString(static_cast<eResourceType>(entry->GetAssetType())));
		}

		const auto offset = NarrowTo<std::size_t>(entry->GetOffset());
		const auto size = NarrowTo<std::size_t>(entry->GetSize());
		if (!offset || !size)
		{
			return UNEXPECTED("Диапазон ресурса с GUID '{}' не представим адресным размером платформы", guid.ToString());
		}

		return AssetLocation{
			.location = eFileLocation::App,
			.relativePath = { c_DataPackageRelativePath },
			.offset = *offset,
			.size = *size,
			.entry = { entry }
		};
	}

	void DataAssetsManager::LogDataEntriesSummary(const DatFileHeader& header) const
	{
#if Z_ADD_LOGGER
		DOut("========== [DataAssetsManager] Data Package: {} (Total entries: {}) ==========",
			c_DataPackageRelativePath.generic_string(), m_Entries.size());
		header.LogFileBlock("  ");
		for (const auto& [guid, entry] : m_Entries)
		{
			DOut("  [DataEntry] type: {}, guid: {}, size: {} bytes",
				ToString(static_cast<eResourceType>(entry.GetAssetType())), guid.ToString(), entry.GetSize());
		}
#endif // Z_ADD_LOGGER
	}
}
