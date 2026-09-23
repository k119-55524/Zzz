
#include <type_traits>

#include "core/utils/SafeRange.h"
#include "core/io/DatFileHeader.h"
#include "core/io/package/ArchiveTableReader.h"
#include "core/constants/PackageConstants.h"

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
		auto tableRes = ReadArchiveTable(
			fileSystem,
			eFileLocation::App,
			c_DataPackageRelativePath,
			c_DataDatHeader,
			c_DataDatFileMajorVersion,
			[](zU32 assetType) noexcept
			{
				const auto rawType = NarrowTo<std::underlying_type_t<eResourceType>>(assetType);
				return rawType && IsDataArchiveResourceType(static_cast<eResourceType>(*rawType));
			});
		if (!tableRes)
			THROW_RUNTIME("Ошибка загрузки архива данных '{}': {}", c_DataPackageRelativePath.generic_string(), tableRes.error());

		m_Entries.clear();
		m_Entries.reserve(tableRes->entries.size());
		for (const auto& entry : tableRes->entries)
		{
			m_Entries.emplace(entry.GetGuid(), entry);
		}

		const DatFileHeader& header = tableRes->header;
		LogDataEntriesSummary(header);
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
			.relativePath = std::cref(c_DataPackageRelativePath),
			.entry = std::cref(*entry)
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
