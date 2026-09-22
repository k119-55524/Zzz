
#include "core/io/DatFileHeader.h"
#include "core/io/package/SceneData.h"
#include "core/io/package/PrefabData.h"
#include "core/constants/PackageConstants.h"
#include "core/io/package/ChildViewData.h"
#include "core/io/package/PrimaryViewData.h"
#include "core/io/package/ProjectManifestData.h"
#include "core/io/package/IndependentViewData.h"

#include "PackageManager.h"

namespace zzz::engine
{
	PackageManager::PackageManager(std::shared_ptr<FileSystem> fileSystem)
		: m_FileSystem(std::move(fileSystem))
	{
		ensure(m_FileSystem, "FileSystem не должен быть null при создании PackageManager.");
		Initialize();
	}

	void PackageManager::Initialize()
	{
		auto headerBufferRes = m_FileSystem->ReadBytes(eFileLocation::App, c_GamePackageRelativePath, 0, DatFileHeader::BinarySize());
		if (!headerBufferRes)
			THROW_RUNTIME("Отсутствует обязательный пакет игровых ресурсов: {}: {}", c_GamePackageRelativePath.generic_string(), headerBufferRes.error());

		std::size_t offset = 0;
		Serializer serializer;
		auto headerRes = serializer.Deserialize(*headerBufferRes, offset, m_Header);
		if (!headerRes)
			THROW_RUNTIME("Ошибка десериализации заголовка пакета '{}': {}", c_GamePackageRelativePath.generic_string(), headerRes.error());

		auto validRes = m_Header.Validate(c_PackageDatHeader, c_PackageDatFileMajorVersion);
		if (!validRes)
			THROW_RUNTIME("Некорректный заголовок в файле '{}': {}", c_GamePackageRelativePath.generic_string(), validRes.error());

		m_EntriesByGuid.clear();

		const zU32 entryCount = m_Header.GetEntryCount();
		if (entryCount > 0)
		{
			const std::size_t tableSize = static_cast<std::size_t>(entryCount) * PackageEntry::BinarySize();
			auto tableBufferRes = m_FileSystem->ReadBytes(eFileLocation::App, c_GamePackageRelativePath, DatFileHeader::BinarySize(), tableSize);
			if (!tableBufferRes)
				THROW_RUNTIME("Ошибка чтения таблицы записей пакета '{}': {}", c_GamePackageRelativePath.generic_string(), tableBufferRes.error());

			std::size_t tableOffset = 0;
			for (zU32 i = 0; i < entryCount; ++i)
			{
				PackageEntry entry{};
				auto entryRes = serializer.Deserialize(*tableBufferRes, tableOffset, entry);
				if (!entryRes)
					THROW_RUNTIME("Ошибка десериализации записи пакета #{} в файле '{}': {}", i, c_GamePackageRelativePath.generic_string(), entryRes.error());

				auto type = static_cast<ePackage>(entry.GetAssetType());
#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
				ensure(entry.GetGuid().IsValid(), "Ресурс в пакете package.dat имеет невалидный (нулевой) GUID!");
				ensure(!m_EntriesByGuid[type].contains(entry.GetGuid()), "Обнаружен дубликат GUID {} ресурса типа {} в package.dat!", entry.GetGuid().ToString(), ToString(type));
#endif
				m_EntriesByGuid[type].emplace(entry.GetGuid(), entry);
			}
		}

		auto primaryViewIt = m_EntriesByGuid.find(ePackage::PrimaryView);
		if (primaryViewIt == m_EntriesByGuid.end() || primaryViewIt->second.empty())
			THROW_RUNTIME("Ошибка пакета '{}': Обязательный ресурс PrimaryViewData отсутствует.", c_GamePackageRelativePath.generic_string());

		if (primaryViewIt->second.size() > 1)
			THROW_RUNTIME("Ошибка пакета '{}': Ресурс PrimaryViewData не уникален (найдено {} штук).", c_GamePackageRelativePath.generic_string(), primaryViewIt->second.size());

		auto manifestIt = m_EntriesByGuid.find(ePackage::ProjectManifest);
		if (manifestIt == m_EntriesByGuid.end() || manifestIt->second.empty())
			THROW_RUNTIME("Ошибка пакета '{}': Обязательный ресурс ProjectManifestData отсутствует.", c_GamePackageRelativePath.generic_string());

		if (manifestIt->second.size() > 1)
			THROW_RUNTIME("Ошибка пакета '{}': Ресурс ProjectManifestData не уникален (найдено {} штук).", c_GamePackageRelativePath.generic_string(), manifestIt->second.size());

		auto manifestRes = DeserializeEntry<ProjectManifestData>(manifestIt->second.begin()->second);
		if (!manifestRes)
			THROW_RUNTIME("Ошибка десериализации ProjectManifestData из пакета '{}': {}", c_GamePackageRelativePath.generic_string(), manifestRes.error());

		if (manifestRes->GetCompanyName().empty() || manifestRes->GetAppName().empty())
			THROW_RUNTIME("Ошибка пакета '{}': ProjectManifestData не содержит имя компании и/или приложения.", c_GamePackageRelativePath.generic_string());

		m_CompanyName = manifestRes->GetCompanyName();
		m_AppName = manifestRes->GetAppName();
		m_ProjectManifest = std::move(*manifestRes);

		LogPackageEntriesSummary();
	}

	[[nodiscard]] std::expected<PrimaryViewData, std::string> PackageManager::GetPrimaryViewData() const
	{
		auto it = m_EntriesByGuid.find(ePackage::PrimaryView);
		if (it == m_EntriesByGuid.end() || it->second.empty())
			return UNEXPECTED("Package entry of type PrimaryView was not found.");

		return DeserializeEntry<PrimaryViewData>(it->second.begin()->second);
	}

	[[nodiscard]] std::optional<PackageEntry> PackageManager::GetEntry(ePackage type, const Guid& guid) const
	{
		auto it = m_EntriesByGuid.find(type);
		if (it == m_EntriesByGuid.end())
			return std::nullopt;

		auto guidIt = it->second.find(guid);
		if (guidIt == it->second.end())
			return std::nullopt;

		return guidIt->second;
	}

	std::expected<std::vector<std::byte>, std::string> PackageManager::ReadRawBytes(const PackageEntry& entry) const
	{
		auto bufferRes = m_FileSystem->ReadBytes(eFileLocation::App, c_GamePackageRelativePath, entry.GetOffset(), entry.GetSize());
		if (!bufferRes)
		{
			return UNEXPECTED("Не удалось прочитать блок данных с GUID '{}' из пакета '{}': {}",
				entry.GetGuid().ToString(), c_GamePackageRelativePath.generic_string(), bufferRes.error());
		}
		return bufferRes;
	}

	template <typename T> requires std::derived_from<T, ISerializable>
	[[nodiscard]] std::expected<T, std::string> PackageManager::DeserializeEntryFromMemory(const PackageEntry& entry, std::span<const std::byte> bytes)
	{
		std::size_t offset = 0;
		Serializer serializer;
		T data{};
		auto res = serializer.Deserialize(bytes, offset, data);
		if (!res)
			return UNEXPECTED("Ошибка десериализации данных пакета с GUID '{}': {}.", entry.GetGuid().ToString(), res.error());

		return data;
	}

	template <typename T> requires std::derived_from<T, ISerializable>
	[[nodiscard]] std::expected<T, std::string> PackageManager::DeserializeEntry(const PackageEntry& entry) const
	{
		auto bufferRes = ReadRawBytes(entry);
		if (!bufferRes)
			return UNEXPECTED("{}", bufferRes.error());

		return DeserializeEntryFromMemory<T>(entry, *bufferRes);
	}

	template std::expected<ProjectManifestData, std::string> PackageManager::DeserializeEntryFromMemory<ProjectManifestData>(const PackageEntry&, std::span<const std::byte>);
	template std::expected<PrimaryViewData, std::string> PackageManager::DeserializeEntryFromMemory<PrimaryViewData>(const PackageEntry&, std::span<const std::byte>);
	template std::expected<SceneData, std::string> PackageManager::DeserializeEntryFromMemory<SceneData>(const PackageEntry&, std::span<const std::byte>);
	template std::expected<ChildViewData, std::string> PackageManager::DeserializeEntryFromMemory<ChildViewData>(const PackageEntry&, std::span<const std::byte>);
	template std::expected<IndependentViewData, std::string> PackageManager::DeserializeEntryFromMemory<IndependentViewData>(const PackageEntry&, std::span<const std::byte>);
	template std::expected<PrefabData, std::string> PackageManager::DeserializeEntryFromMemory<PrefabData>(const PackageEntry&, std::span<const std::byte>);

	template std::expected<ProjectManifestData, std::string> PackageManager::DeserializeEntry<ProjectManifestData>(const PackageEntry&) const;
	template std::expected<PrimaryViewData, std::string> PackageManager::DeserializeEntry<PrimaryViewData>(const PackageEntry&) const;
	template std::expected<SceneData, std::string> PackageManager::DeserializeEntry<SceneData>(const PackageEntry&) const;
	template std::expected<ChildViewData, std::string> PackageManager::DeserializeEntry<ChildViewData>(const PackageEntry&) const;
	template std::expected<IndependentViewData, std::string> PackageManager::DeserializeEntry<IndependentViewData>(const PackageEntry&) const;
	template std::expected<PrefabData, std::string> PackageManager::DeserializeEntry<PrefabData>(const PackageEntry&) const;

#pragma region Logging
	void PackageManager::LogPackageEntriesSummary() const
	{
#if Z_ADD_LOGGER
		DOut("========== [PackageManager] Package Data: {} ==========", c_GamePackageRelativePath.generic_string());
		m_Header.LogFileBlock("  ");
		// Закомментируй тот тип ресурса, который не хочешь логировать
		LogEntriesSummaryForType<ProjectManifestData>(ePackage::ProjectManifest);
		LogEntriesSummaryForType<PrimaryViewData>(ePackage::PrimaryView);
		LogEntriesSummaryForType<SceneData>(ePackage::Scene);
		LogEntriesSummaryForType<ChildViewData>(ePackage::ChildView);
		LogEntriesSummaryForType<IndependentViewData>(ePackage::IndependentView);
		LogEntriesSummaryForType<PrefabData>(ePackage::Prefab);
#endif
	}

	template <typename T> requires std::derived_from<T, ISerializable>
	void PackageManager::LogEntriesSummaryForType(ePackage type) const
	{
		auto typeIt = m_EntriesByGuid.find(type);
		const size_t count = (typeIt != m_EntriesByGuid.end()) ? typeIt->second.size() : 0;
		DOut("  [PackageType] {}({})", ToString(type), count);

		if (typeIt == m_EntriesByGuid.end() || typeIt->second.empty())
		{
			DOut("");
			return;
		}

		const auto& entriesMap = typeIt->second;
		for (const auto& [guid, entry] : entriesMap)
		{
			entry.LogFileBlock("    ");

			if (auto dataRes = DeserializeEntry<T>(entry))
			{
				dataRes->LogFileBlock("      ");
			}
		}
		DOut("");
	}
#pragma endregion
}
