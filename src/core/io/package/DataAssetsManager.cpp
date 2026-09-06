#include "core/io/package/DataAssetsManager.h"
#include "core/io/package/PackageHeader.h"
#include "core/io/package/MeshData.h"
#include "core/constants/PackageConstants.h"
#include "core/utils/ThrowWrappers.h"
#include <logger/logger.h>

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
		// 1. Читаем строго заголовок архива данных
		constexpr std::size_t headerSize = PackageHeader::BinarySize();
		auto headerBufferRes = m_FileSystem->ReadBytes(eFileLocation::App, c_DataPackageRelativePath, 0, headerSize);
		if (!headerBufferRes)
			THROW_RUNTIME("Отсутствует обязательный архив игровых ресурсов: {}: {}", c_DataPackageRelativePath, headerBufferRes.error());

		std::size_t offset = 0;
		Serializer serializer;
		PackageHeader header;
		auto headerRes = serializer.Deserialize(*headerBufferRes, offset, header);
		if (!headerRes)
			THROW_RUNTIME("Ошибка десериализации заголовка архива данных '{}': {}", c_DataPackageRelativePath, headerRes.error());

		auto validRes = header.Validate(c_DataPackageHeader, c_DataPackageFileMajorVersion);
		if (!validRes)
			THROW_RUNTIME("Некорректный заголовок в файле '{}': {}", c_DataPackageRelativePath, validRes.error());

		m_EntriesByGuid.clear();
		m_EntriesByName.clear();

		// 2. Читаем ровно оглавление (TOC) благодаря фиксированному размеру PackageEntry
		const std::size_t tableSize = header.GetEntryCount() * PackageEntry::BinarySize();
		if (tableSize > 0)
		{
			auto tableBufferRes = m_FileSystem->ReadBytes(eFileLocation::App, c_DataPackageRelativePath, headerSize, tableSize);
			if (!tableBufferRes)
				THROW_RUNTIME("Не удалось прочитать таблицу записей архива данных '{}': {}", c_DataPackageRelativePath, tableBufferRes.error());

			std::size_t tableOffset = 0;
			for (zU32 i = 0; i < header.GetEntryCount(); ++i)
			{
				PackageEntry entry{};
				auto entryRes = serializer.Deserialize(*tableBufferRes, tableOffset, entry);
				if (!entryRes)
					THROW_RUNTIME("Ошибка десериализации записи архива данных #{} в файле '{}': {}", i, c_DataPackageRelativePath, entryRes.error());

				m_EntriesByGuid[entry.GetGuid()] = entry;
				m_EntriesByName[std::string(entry.GetName())] = entry;
			}
		}

		LogDataEntriesSummary();
	}

	void DataAssetsManager::LogDataEntriesSummary() const
	{
#if Z_ADD_LOGGER
		DOut("========== [DataAssetsManager] Data Package: {} (Total entries: {}) ==========",
			c_DataPackageRelativePath, m_EntriesByGuid.size());
		for (const auto& [guid, entry] : m_EntriesByGuid)
		{
			const auto resType = static_cast<eResourceType>(entry.GetAssetType());
			DOut("  [DataEntry] type: {}, guid: {}, name: '{}', size: {} bytes",
				ToString(resType), guid.ToString(), entry.GetName(), entry.GetSize());
		}
#endif
	}

	template std::expected<MeshData, std::string> DataAssetsManager::LoadData<MeshData>(eResourceType, const Guid&) const;
	template std::expected<MeshData, std::string> DataAssetsManager::LoadData<MeshData>(const PackageEntry&) const;
}
