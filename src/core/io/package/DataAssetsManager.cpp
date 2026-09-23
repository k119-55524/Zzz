
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

		m_EntriesByGuid.clear();

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

				auto type = static_cast<eResourceType>(entry.GetAssetType());
#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
				ensure(entry.GetGuid().IsValid(), "Ресурс в пакете data.dat имеет невалидный (нулевой) GUID!");
				ensure(!m_EntriesByGuid[type].contains(entry.GetGuid()), "Обнаружен дубликат GUID {} ресурса типа {} в data.dat!", entry.GetGuid().ToString(), ToString(type));
#endif
				m_EntriesByGuid[type].emplace(entry.GetGuid(), entry);
			}
		}

		LogDataEntriesSummary();
	}

	[[nodiscard]] const PackageEntry* DataAssetsManager::GetEntryPtr(eResourceType type, const Guid& guid) const
	{
		auto it = m_EntriesByGuid.find(type);
		if (it == m_EntriesByGuid.end())
			return nullptr;

		auto guidIt = it->second.find(guid);
		if (guidIt == it->second.end())
			return nullptr;

		return &guidIt->second;
	}

	[[nodiscard]] std::expected<AssetLocation, std::string> DataAssetsManager::GetAssetLocation(eResourceType type, const Guid& guid) const
	{
		const PackageEntry* entry = GetEntryPtr(type, guid);
		if (!entry)
		{
			return UNEXPECTED("Ресурс типа {} с GUID '{}' не найден в оглавлении data.dat",
				ToString(type), guid.ToString());
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
		size_t totalCount = 0;
		for (const auto& [type, entries] : m_EntriesByGuid)
			totalCount += entries.size();

		DOut("========== [DataAssetsManager] Data Package: {} (Total entries: {}) ==========",
			c_DataPackageRelativePath.generic_string(), totalCount);
		m_Header.LogFileBlock("  ");
		for (const auto& [type, entries] : m_EntriesByGuid)
		{
			for (const auto& [guid, entry] : entries)
			{
				DOut("  [DataEntry] type: {}, guid: {}, size: {} bytes",
					ToString(type), guid.ToString(), entry.GetSize());
			}
		}
#endif // Z_ADD_LOGGER
	}
}
