#include "core/io/package/SceneData.h"
#include "core/io/package/PrefabData.h"
#include "core/io/package/PackageHeader.h"
#include "core/io/package/ChildViewData.h"
#include "core/io/package/IndependentViewData.h"
#include "core/constants/PackageConstants.h"
#include "core/utils/Ensure.h"

#include "PackageManager.h"

Z_SET_LOG_CATEGORY(::zzz::core::Assets);

using namespace zzz::core;

namespace zzz::engine
{
	PackageManager::PackageManager(std::shared_ptr<FileSystem> fileSystem) :
		m_FileSystem{ std::move(fileSystem) }
	{
		ensure(m_FileSystem, "FileSystem не должен быть null при создании PackageManager.");
		Initialize();
	}

	void PackageManager::Initialize()
	{
		auto fileBufferRes = m_FileSystem->ReadAllBytes(eFileLocation::App, c_GamePackageRelativePath);
		if (!fileBufferRes)
			THROW_RUNTIME("Не удалось прочитать файл пакета '{}': {}", c_GamePackageRelativePath, fileBufferRes.error());

		const auto& fileBuffer = *fileBufferRes;
		std::size_t offset = 0;
		Serializer serializer;
		PackageHeader header;
		auto headerRes = serializer.Deserialize(fileBuffer, offset, header);
		if (!headerRes)
			THROW_RUNTIME("Ошибка десериализации заголовка пакета '{}': {}", c_GamePackageRelativePath, headerRes.error());

		auto validRes = header.Validate();
		if (!validRes)
			THROW_RUNTIME("Некорректный заголовок в файле '{}': {}", c_GamePackageRelativePath, validRes.error());

		m_EntriesByName.clear();
		m_EntriesByGuid.clear();

		for (zU32 i = 0; i < header.GetEntryCount(); ++i)
		{
			PackageEntry entry{};
			auto entryRes = serializer.Deserialize(fileBuffer, offset, entry);
			if (!entryRes)
				THROW_RUNTIME("Ошибка десериализации записи пакета #{} в файле '{}': {}", i, c_GamePackageRelativePath, entryRes.error());

			auto type = static_cast<ePackage>(entry.GetAssetType());
			m_EntriesByName[type][entry.GetName()] = entry;
			m_EntriesByGuid[type][entry.GetGuid()] = entry;
		}

		auto primaryViewIt = m_EntriesByName.find(ePackage::PrimaryView);
		if (primaryViewIt == m_EntriesByName.end() || primaryViewIt->second.empty())
			THROW_RUNTIME("Ошибка пакета '{}': Обязательный ресурс PrimaryViewData отсутствует.", c_GamePackageRelativePath);

		if (primaryViewIt->second.size() > 1)
			THROW_RUNTIME("Ошибка пакета '{}': Ресурс PrimaryViewData не уникален (найдено {} штук).", c_GamePackageRelativePath, primaryViewIt->second.size());

		auto manifestIt = m_EntriesByName.find(ePackage::ProjectManifest);
		if (manifestIt == m_EntriesByName.end() || manifestIt->second.empty())
			THROW_RUNTIME("Ошибка пакета '{}': Обязательный ресурс ProjectManifestData отсутствует.", c_GamePackageRelativePath);

		if (manifestIt->second.size() > 1)
			THROW_RUNTIME("Ошибка пакета '{}': Ресурс ProjectManifestData не уникален (найдено {} штук).", c_GamePackageRelativePath, manifestIt->second.size());

		auto manifestRes = LoadPackageData<ProjectManifestData>(manifestIt->second.begin()->second);
		if (!manifestRes)
			THROW_RUNTIME("Ошибка десериализации ProjectManifestData из пакета '{}': {}", c_GamePackageRelativePath, manifestRes.error());

		if (manifestRes->GetCompanyName().empty() || manifestRes->GetAppName().empty())
			THROW_RUNTIME("Ошибка пакета '{}': ProjectManifestData не содержит имя компании и/или приложения.", c_GamePackageRelativePath);

		m_CompanyName = manifestRes->GetCompanyName();
		m_AppName = manifestRes->GetAppName();
		m_ProjectManifest = std::move(*manifestRes);

		LogPackageEntriesSummary();
	}

	[[nodiscard]] std::expected<PrimaryViewData, std::string> PackageManager::GetPrimaryViewData() const
	{
		auto it = m_EntriesByName.find(ePackage::PrimaryView);
		if (it == m_EntriesByName.end() || it->second.empty())
			return UNEXPECTED("Package entry of type PrimaryView was not found.");

		return LoadPackageData<PrimaryViewData>(it->second.begin()->second);
	}

	[[nodiscard]] std::optional<PackageEntry> PackageManager::GetEntryByName(ePackage type, std::string_view name) const
	{
		auto it = m_EntriesByName.find(type);
		if (it == m_EntriesByName.end())
			return std::nullopt;

		auto nameIt = it->second.find(std::string(name));
		if (nameIt == it->second.end())
			return std::nullopt;

		return nameIt->second;
	}

	[[nodiscard]] std::optional<PackageEntry> PackageManager::GetEntryByGuid(ePackage type, const Guid& guid) const
	{
		auto it = m_EntriesByGuid.find(type);
		if (it == m_EntriesByGuid.end())
			return std::nullopt;

		auto guidIt = it->second.find(guid);
		if (guidIt == it->second.end())
			return std::nullopt;

		return guidIt->second;
	}

	template <typename T> requires std::derived_from<T, ISerializable>
	[[nodiscard]] std::expected<T, std::string> PackageManager::LoadPackageData(const PackageEntry& entry) const
	{
		auto bufferRes = m_FileSystem->ReadBytes(eFileLocation::App, c_GamePackageRelativePath, entry.GetOffset(), entry.GetSize());
		if (!bufferRes)
			return UNEXPECTED("Не удалось прочитать блок данных '{}' из пакета '{}': {}",
				entry.GetName(), c_GamePackageRelativePath, bufferRes.error());

		const auto& buffer = *bufferRes;
		std::size_t offset = 0;
		Serializer serializer;
		T data{};
		auto res = serializer.Deserialize(buffer, offset, data);
		if (!res)
			return UNEXPECTED("Ошибка десериализации данных пакета '{}': {}.", entry.GetName(), res.error());

		if constexpr (std::is_same_v<T, ChildViewData> || std::is_same_v<T, IndependentViewData>)
		{
			// Reserved for name tagging if needed
		}
		return data;
	}

	template std::expected<ProjectManifestData, std::string> PackageManager::LoadPackageData<ProjectManifestData>(const PackageEntry&) const;
	template std::expected<PrimaryViewData, std::string> PackageManager::LoadPackageData<PrimaryViewData>(const PackageEntry&) const;
	template std::expected<SceneData, std::string> PackageManager::LoadPackageData<SceneData>(const PackageEntry&) const;
	template std::expected<ChildViewData, std::string> PackageManager::LoadPackageData<ChildViewData>(const PackageEntry&) const;
	template std::expected<IndependentViewData, std::string> PackageManager::LoadPackageData<IndependentViewData>(const PackageEntry&) const;
	template std::expected<PrefabData, std::string> PackageManager::LoadPackageData<PrefabData>(const PackageEntry&) const;

#pragma region Logging
	void PackageManager::LogPackageEntriesSummary() const
	{
#if Z_ADD_LOGGER
		DOut("========== [PackageManager] Package Data: {} ==========", c_GamePackageRelativePath);
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
		auto typeIt = m_EntriesByName.find(type);
		const size_t count = (typeIt != m_EntriesByName.end()) ? typeIt->second.size() : 0;
		DOut("  [PackageType] {}({})", ToString(type), count);

		if (typeIt == m_EntriesByName.end() || typeIt->second.empty())
		{
			DOut("");
			return;
		}

		const auto& entriesMap = typeIt->second;
		for (const auto& entryPair : entriesMap)
		{
			const auto& entry = entryPair.second;
			entry.LogFileBlock("    ");

			if (auto dataRes = LoadPackageData<T>(entry))
			{
				dataRes->LogFileBlock("      ");
			}
		}
		DOut("");
	}
#pragma endregion
}
