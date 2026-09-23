
#include "core/io/DatFileHeader.h"
#include "core/constants/PackageConstants.h"

#include "DataAssetsManager.h"


Z_SET_LOG_CATEGORY(::zzz::core::Assets);

namespace zzz::core
{
	DataAssetsManager::DataAssetsManager(std::shared_ptr<FileSystem> fileSystem)
		: m_FileSystem(std::move(fileSystem))
	{
		ensure(m_FileSystem, "FileSystem не должен быть null при создании DataAssetsManager.");
		Initialize();
	}

	void DataAssetsManager::Initialize()
	{
		auto headerBufferRes = m_FileSystem->ReadBytes(eFileLocation::App, c_DataPackageRelativePath, 0, DatFileHeader::BinarySize());
		if (!headerBufferRes)
			THROW_RUNTIME("Отсутствует обязательный архив игровых ресурсов: {}: {}", c_DataPackageRelativePath.generic_string(), headerBufferRes.error());

		std::size_t offset = 0;
		Serializer serializer;
		auto headerRes = serializer.Deserialize(*headerBufferRes, offset, m_Header);
		if (!headerRes)
			THROW_RUNTIME("Ошибка десериализации заголовка архива данных '{}': {}", c_DataPackageRelativePath.generic_string(), headerRes.error());

		auto validRes = m_Header.Validate(c_DataDatHeader, c_DataDatFileMajorVersion);
		if (!validRes)
			THROW_RUNTIME("Некорректный заголовок в файле '{}': {}", c_DataPackageRelativePath.generic_string(), validRes.error());

		m_Entries.clear();

		const zU32 entryCount = m_Header.GetEntryCount();
		if (entryCount > 0)
		{
			const std::size_t tableSize = static_cast<std::size_t>(entryCount) * PackageEntry::BinarySize();
			auto tableBufferRes = m_FileSystem->ReadBytes(eFileLocation::App, c_DataPackageRelativePath, DatFileHeader::BinarySize(), tableSize);
			if (!tableBufferRes)
				THROW_RUNTIME("Ошибка чтения таблицы записей архива данных '{}': {}", c_DataPackageRelativePath.generic_string(), tableBufferRes.error());

			std::size_t tableOffset = 0;
			for (zU32 i = 0; i < entryCount; ++i)
			{
				PackageEntry entry{};
				auto entryRes = serializer.Deserialize(*tableBufferRes, tableOffset, entry);
				if (!entryRes)
					THROW_RUNTIME("Ошибка десериализации записи архива данных #{} в файле '{}': {}", i, c_DataPackageRelativePath.generic_string(), entryRes.error());

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

		LogDataEntriesSummary();
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

		return AssetLocation{
			.location = eFileLocation::App,
			.relativePath = c_DataPackageRelativePath,
			.offset = entry->GetOffset(),
			.size = entry->GetSize(),
			.entry = entry
		};
	}

	void DataAssetsManager::LogDataEntriesSummary() const
	{
#if Z_ADD_LOGGER
		DOut("========== [DataAssetsManager] Data Package: {} (Total entries: {}) ==========",
			c_DataPackageRelativePath.generic_string(), m_Entries.size());
		m_Header.LogFileBlock("  ");
		for (const auto& [guid, entry] : m_Entries)
		{
			DOut("  [DataEntry] type: {}, guid: {}, size: {} bytes",
				ToString(static_cast<eResourceType>(entry.GetAssetType())), guid.ToString(), entry.GetSize());
		}
#endif // Z_ADD_LOGGER
	}
}
