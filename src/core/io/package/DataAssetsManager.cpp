#include "DataAssetsManager.h"
#include "core/logger/logger.h"
#include "core/io/DatFileHeader.h"
#include "core/io/package/MeshData.h"
#include "core/io/package/MaterialData.h"
#include "core/constants/PackageConstants.h"

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
			THROW_RUNTIME("Отсутствует обязательный архив игровых ресурсов: {}: {}", c_DataPackageRelativePath, headerBufferRes.error());

		std::size_t offset = 0;
		Serializer serializer;
		auto headerRes = serializer.Deserialize(*headerBufferRes, offset, m_Header);
		if (!headerRes)
			THROW_RUNTIME("Ошибка десериализации заголовка архива данных '{}': {}", c_DataPackageRelativePath, headerRes.error());

		auto validRes = m_Header.Validate(c_DataPackageHeader, c_DataPackageFileMajorVersion);
		if (!validRes)
			THROW_RUNTIME("Некорректный заголовок в файле '{}': {}", c_DataPackageRelativePath, validRes.error());

		m_EntriesByGuid.clear();
		m_EntriesByName.clear();

		const zU32 entryCount = m_Header.GetEntryCount();
		if (entryCount > 0)
		{
			const std::size_t tableSize = static_cast<std::size_t>(entryCount) * PackageEntry::BinarySize();
			auto tableBufferRes = m_FileSystem->ReadBytes(eFileLocation::App, c_DataPackageRelativePath, DatFileHeader::BinarySize(), tableSize);
			if (!tableBufferRes)
				THROW_RUNTIME("Ошибка чтения таблицы записей архива данных '{}': {}", c_DataPackageRelativePath, tableBufferRes.error());

			std::size_t tableOffset = 0;
			for (zU32 i = 0; i < entryCount; ++i)
			{
				PackageEntry entry{};
				auto entryRes = serializer.Deserialize(*tableBufferRes, tableOffset, entry);
				if (!entryRes)
					THROW_RUNTIME("Ошибка десериализации записи архива данных #{} в файле '{}': {}", i, c_DataPackageRelativePath, entryRes.error());

				auto type = static_cast<eResourceType>(entry.GetAssetType());
				m_EntriesByGuid[type][entry.GetGuid()] = entry;
				m_EntriesByName[type][std::string(entry.GetName())] = entry;
			}
		}

		LogDataEntriesSummary();
	}

	[[nodiscard]] std::optional<PackageEntry> DataAssetsManager::GetEntry(eResourceType type, std::string_view name) const
	{
		auto it = m_EntriesByName.find(type);
		if (it == m_EntriesByName.end())
			return std::nullopt;

		auto nameIt = it->second.find(std::string(name));
		if (nameIt == it->second.end())
			return std::nullopt;

		return nameIt->second;
	}

	[[nodiscard]] std::optional<PackageEntry> DataAssetsManager::GetEntry(eResourceType type, const Guid& guid) const
	{
		auto it = m_EntriesByGuid.find(type);
		if (it == m_EntriesByGuid.end())
			return std::nullopt;

		auto guidIt = it->second.find(guid);
		if (guidIt == it->second.end())
			return std::nullopt;

		return guidIt->second;
	}

	void DataAssetsManager::LogDataEntriesSummary() const
	{
#if Z_ADD_LOGGER
		size_t totalCount = 0;
		for (const auto& [type, entries] : m_EntriesByGuid)
			totalCount += entries.size();

		DOut("========== [DataAssetsManager] Data Package: {} (Total entries: {}) ==========",
			c_DataPackageRelativePath, totalCount);
		m_Header.LogFileBlock("  ");
		for (const auto& [type, entries] : m_EntriesByGuid)
		{
			for (const auto& [guid, entry] : entries)
			{
				DOut("  [DataEntry] type: {}, guid: {}, name: '{}', size: {} bytes",
					ToString(type), guid.ToString(), entry.GetName(), entry.GetSize());
			}
		}
#endif // Z_ADD_LOGGER
	}

	std::expected<std::vector<std::byte>, std::string> DataAssetsManager::ReadRawBytes(const PackageEntry& entry) const
	{
		auto bufferRes = m_FileSystem->ReadBytes(eFileLocation::App, c_DataPackageRelativePath, entry.GetOffset(), entry.GetSize());
		if (!bufferRes)
		{
			return UNEXPECTED("Не удалось прочитать блок данных '{}' из пакета '{}': {}",
				entry.GetName(), c_DataPackageRelativePath, bufferRes.error());
		}
		return bufferRes;
	}

	template std::expected<MeshData, std::string> DataAssetsManager::LoadAsset<MeshData>(const Guid&) const;
	template std::expected<MeshData, std::string> DataAssetsManager::LoadAsset<MeshData>(std::string_view) const;
	template std::expected<MeshData, std::string> DataAssetsManager::DeserializeEntry<MeshData>(const PackageEntry&) const;

	template std::expected<MaterialData, std::string> DataAssetsManager::LoadAsset<MaterialData>(const Guid&) const;
	template std::expected<MaterialData, std::string> DataAssetsManager::LoadAsset<MaterialData>(std::string_view) const;
	template std::expected<MaterialData, std::string> DataAssetsManager::DeserializeEntry<MaterialData>(const PackageEntry&) const;
}
