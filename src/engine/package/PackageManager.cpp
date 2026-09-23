
#include "core/utils/Ensure.h"
#include "core/utils/SafeMath.h"
#include "core/io/DatFileHeader.h"
#include "core/io/package/PackageEntry.h"
#include "core/io/package/scene/SceneData.h"
#include "core/constants/PackagesConstants.h"
#include "core/io/package/assets/PrefabData.h"
#include "core/io/package/ProjectManifestData.h"
#include "core/io/package/views/ChildViewData.h"
#include "core/io/package/views/PrimaryViewData.h"
#include "core/io/package/views/IndependentViewData.h"

#include "PackageManager.h"

Z_SET_LOG_CATEGORY(::zzz::core::Assets);

using namespace zzz::core;

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
		const std::string pathStr = c_GamePackageRelativePath.generic_string();

		auto fileSizeRes = m_FileSystem->GetFileSize(eFileLocation::App, c_GamePackageRelativePath);
		if (!fileSizeRes)
			THROW_RUNTIME("Ошибка получения размера файла '{}': {}", pathStr, fileSizeRes.error());

		const std::uintmax_t fileSize = *fileSizeRes;

		auto headerBytesRes = m_FileSystem->ReadBytes(eFileLocation::App, c_GamePackageRelativePath, 0, m_Header.c_BaseHeaderSize);
		if (!headerBytesRes)
			THROW_RUNTIME("Ошибка чтения заголовка пакета '{}': {}", pathStr, headerBytesRes.error());

		if (auto res = m_Header.DeserializeAndValidate(*headerBytesRes, fileSize); !res)
			THROW_RUNTIME("Ошибка заголовка пакета '{}': {}", pathStr, res.error());

		const std::size_t headerSize = m_Header.GetHeaderSize();

		m_EntriesByGuid.clear();
		m_SceneGuidsByName.clear();
		const zU32 entryCount = m_Header.GetEntryCount();
		if (entryCount > 0)
		{
			const auto tableSize = PackageEntry::CalculateTableSize(entryCount);
			if (!tableSize)
				THROW_RUNTIME("Размер таблицы записей пакета '{}' переполняет std::size_t (записей: {})", pathStr, entryCount);

			const std::uintmax_t payloadBegin = headerSize + static_cast<std::uintmax_t>(*tableSize);
			if (!IsRangeInside<std::uintmax_t>(headerSize, *tableSize, fileSize))
				THROW_RUNTIME("Таблица записей пакета '{}' выходит за границы файла: {} > {}", pathStr, payloadBegin, fileSize);

			auto tableBufferRes = m_FileSystem->ReadBytes(eFileLocation::App, c_GamePackageRelativePath, headerSize, *tableSize);
			if (!tableBufferRes)
				THROW_RUNTIME("Ошибка чтения таблицы записей пакета '{}': {}", pathStr, tableBufferRes.error());

			const std::uintmax_t payloadSize = fileSize - payloadBegin;
			std::size_t tableOffset = 0;
			Serializer serializer;

			for (zU32 i = 0; i < entryCount; ++i)
			{
				PackageEntry entry{};
				if (auto res = serializer.Deserialize(*tableBufferRes, tableOffset, entry); !res)
					THROW_RUNTIME("Ошибка десериализации записи #{} пакета '{}': {}", i, pathStr, res.error());

				// Проверка типа ресурса записи
				const auto rawType = entry.GetAssetType();
				const auto pkgType = static_cast<ePackage>(rawType);
				switch (pkgType)
				{
				case ePackage::ProjectManifest:
				case ePackage::Scene:
				case ePackage::PrimaryView:
				case ePackage::ChildView:
				case ePackage::IndependentView:
				case ePackage::Prefab:
					break;
				default:
					THROW_RUNTIME("Запись #{} пакета '{}' содержит недопустимый тип ресурса: {}", i, pathStr, rawType);
				}

				if (!entry.IsRangeValid(payloadBegin, payloadSize))
					THROW_RUNTIME("Запись #{} пакета '{}' содержит недопустимый диапазон (offset={}, size={}) при границах данных [{}, {})",
						i, pathStr, entry.GetOffset(), entry.GetSize(), payloadBegin, fileSize);

#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
				ensure(entry.GetGuid().IsValid(), "Запись пакета #{} содержит невалидный GUID.", i);
				if (auto it = m_EntriesByGuid.find(pkgType); it != m_EntriesByGuid.end())
				{
					ensure(!it->second.contains(entry.GetGuid()),
						"Обнаружен дубликат GUID={} для типа {} в пакете '{}'",
						entry.GetGuid().ToString(), static_cast<zU32>(pkgType), pathStr);
				}
#endif

				m_EntriesByGuid[pkgType].emplace(entry.GetGuid(), entry);
			}
		}

		auto primaryViewIt = m_EntriesByGuid.find(ePackage::PrimaryView);
		if (primaryViewIt == m_EntriesByGuid.end() || primaryViewIt->second.empty())
			THROW_RUNTIME("Ошибка пакета '{}': Обязательный ресурс PrimaryViewData отсутствует.", pathStr);

		if (primaryViewIt->second.size() > 1)
			THROW_RUNTIME("Ошибка пакета '{}': Ресурс PrimaryViewData не уникален (найдено {} штук).", pathStr, primaryViewIt->second.size());

		auto manifestIt = m_EntriesByGuid.find(ePackage::ProjectManifest);
		if (manifestIt == m_EntriesByGuid.end() || manifestIt->second.empty())
			THROW_RUNTIME("Ошибка пакета '{}': Обязательный ресурс ProjectManifestData отсутствует.", pathStr);

		if (manifestIt->second.size() > 1)
			THROW_RUNTIME("Ошибка пакета '{}': Ресурс ProjectManifestData не уникален (найдено {} штук).", pathStr, manifestIt->second.size());

		auto manifestRes = DeserializeEntry<ProjectManifestData>(manifestIt->second.begin()->second);
		if (!manifestRes)
			THROW_RUNTIME("Ошибка десериализации ProjectManifestData из пакета '{}': {}", pathStr, manifestRes.error());

		if (manifestRes->GetCompanyName().empty() || manifestRes->GetAppName().empty())
			THROW_RUNTIME("Ошибка пакета '{}': ProjectManifestData не содержит имя компании и/или приложения.", pathStr);

		m_ProjectManifest = std::move(*manifestRes);

		for (const auto& sceneEntry : m_ProjectManifest.GetScenes())
		{
#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
			// Проверка на дубликаты имен сцен в манифесте проекта
			if (auto it = m_SceneGuidsByName.find(sceneEntry.GetName()); it != m_SceneGuidsByName.end())
			{
				ensure(false,
					"Обнаружен дубликат имени сцены '{}' в манифесте проекта package.dat (существующий GUID={}, дублирующий GUID={})!",
					sceneEntry.GetName(),
					it->second.ToString(),
					sceneEntry.GetGuid().ToString());
			}
#endif

			m_SceneGuidsByName.emplace(sceneEntry.GetName(), sceneEntry.GetGuid());
		}

		LogPackageEntriesSummary();
	}

	[[nodiscard]] std::optional<Guid> PackageManager::FindSceneGuidByName(std::string_view name) const noexcept
	{
		auto it = m_SceneGuidsByName.find(name);
		if (it != m_SceneGuidsByName.end())
			return it->second;

		return std::nullopt;
	}

	[[nodiscard]] std::expected<PrimaryViewData, std::string> PackageManager::GetPrimaryViewData() const
	{
		auto it = m_EntriesByGuid.find(ePackage::PrimaryView);
		if (it == m_EntriesByGuid.end() || it->second.empty())
			return UNEXPECTED("Package entry of type PrimaryView was not found.");

		return DeserializeEntry<PrimaryViewData>(it->second.begin()->second);
	}

	[[nodiscard]] const PackageEntry* PackageManager::GetEntry(ePackage type, const Guid& guid) const noexcept
	{
		auto it = m_EntriesByGuid.find(type);
		if (it == m_EntriesByGuid.end())
			return nullptr;

		auto guidIt = it->second.find(guid);
		if (guidIt == it->second.end())
			return nullptr;

		return &guidIt->second;
	}

	std::expected<std::vector<std::byte>, std::string> PackageManager::ReadRawBytes(const PackageEntry& entry) const
	{
		const auto offset = NarrowTo<std::size_t>(entry.GetOffset());
		const auto size = NarrowTo<std::size_t>(entry.GetSize());
		if (!offset || !size)
			return UNEXPECTED("Диапазон ресурса с GUID '{}' не представим адресным размером платформы", entry.GetGuid().ToString());

		auto bufferRes = m_FileSystem->ReadBytes(eFileLocation::App, c_GamePackageRelativePath, *offset, *size);
		if (!bufferRes)
		{
			return UNEXPECTED("Не удалось прочитать блок данных с GUID '{}' из пакета '{}': {}",
				entry.GetGuid().ToString(), c_GamePackageRelativePath.generic_string(), bufferRes.error());
		}
		return bufferRes;
	}

	template <typename T> requires std::derived_from<T, ISerializable>
	[[nodiscard]] std::expected<T, std::string> PackageManager::DeserializeEntry(const PackageEntry& entry) const
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
