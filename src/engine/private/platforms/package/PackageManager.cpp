#include <fstream>
#include <core/io/gamepackage/ViewData.h>
#include <core/io/gamepackage/SceneData.h>
#include <core/io/gamepackage/PrefabData.h>
#include <core/io/gamepackage/ProjectManifestData.h>

#include "PackageManager.h"

using namespace zzz::core;
using namespace zzz::common;

namespace zzz::engine
{
	PackageManager::PackageManager(const Path& path)
	{
		Initialize(path);
	}

	void PackageManager::Initialize(const Path& path)
	{
		auto execDir = path.GetExecutableDirectory();
		if (!execDir)
			THROW_RUNTIME("Не удалось определить путь к бинарному файлу приложения: {}", execDir.error());

		m_PackagePath = *execDir / c_GamePackageFileName;

		if (!std::filesystem::exists(m_PackagePath))
			THROW_RUNTIME("Файл пакета не существует: {}", m_PackagePath.string());

		std::ifstream file(m_PackagePath, std::ios::binary);
		if (!file.is_open())
			THROW_RUNTIME("Не удалось открыть файл пакета: {}", m_PackagePath.string());

		const auto fileSize = std::filesystem::file_size(m_PackagePath);
		std::vector<std::byte> fileBuffer(fileSize);
		file.read(reinterpret_cast<char*>(fileBuffer.data()), fileSize);
		if (!file.good())
			THROW_RUNTIME("Не удалось прочитать файл пакета из: {}", m_PackagePath.string());

		std::size_t offset = 0;
		Serializer serializer;
		PackageHeader header;
		auto headerRes = serializer.Deserialize(fileBuffer, offset, header);
		if (!headerRes)
			THROW_RUNTIME("Ошибка десериализации заголовка пакета {}: {}", m_PackagePath.string(), headerRes.error());

		auto validRes = header.Validate();
		if (!validRes)
			THROW_RUNTIME("Некорректный заголовок в файле {}: {}", m_PackagePath.string(), validRes.error());

		m_EntriesByName.clear();
		m_EntriesByGuid.clear();

		for (zU32 i = 0; i < header.GetEntryCount(); ++i)
		{
			PackageEntry entry{};
			auto entryRes = serializer.Deserialize(fileBuffer, offset, entry);
			if (!entryRes)
				THROW_RUNTIME("Ошибка десериализации записи пакета #{} в файле {}: {}", i, m_PackagePath.string(), entryRes.error());

			auto type = static_cast<ePackage>(entry.assetType);
			m_EntriesByName[type][entry.name] = entry;
			m_EntriesByGuid[type][entry.guid] = entry;
		}

		LogPackageEntriesSummary();
	}

	std::optional<PackageEntry> PackageManager::GetEntryByName(ePackage type, std::string_view name) const
	{
		auto typeIt = m_EntriesByName.find(type);
		if (typeIt == m_EntriesByName.end())
			return std::nullopt;

		auto entryIt = typeIt->second.find(std::string(name));
		if (entryIt == typeIt->second.end())
			return std::nullopt;

		return entryIt->second;
	}

	std::optional<PackageEntry> PackageManager::GetEntryByGuid(ePackage type, const Guid& guid) const
	{
		auto typeIt = m_EntriesByGuid.find(type);
		if (typeIt == m_EntriesByGuid.end())
			return std::nullopt;

		auto entryIt = typeIt->second.find(guid);
		if (entryIt == typeIt->second.end())
			return std::nullopt;

		return entryIt->second;
	}

#pragma region Logging
	void PackageManager::LogPackageEntriesSummary() const
	{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
		// Закомментируй тот тип ресурса, который не хочешь логировать
		LogEntriesSummaryForType<ProjectManifestData>(ePackage::ProjectManifest);
		LogEntriesSummaryForType<SceneData>(ePackage::Scene);
		LogEntriesSummaryForType<ViewData>(ePackage::View);
		LogEntriesSummaryForType<PrefabData>(ePackage::Prefab);
#endif
	}

	template <typename T> requires std::derived_from<T, ISerializable>
	void PackageManager::LogEntriesSummaryForType(ePackage type) const
	{
		auto typeIt = m_EntriesByName.find(type);
		const size_t count = (typeIt != m_EntriesByName.end()) ? typeIt->second.size() : 0;
		DOut("  -> AssetType: {}: {} штук", EnumToString::ToString(type), count);

		if (typeIt == m_EntriesByName.end() || typeIt->second.empty())
			return;

		const auto& entriesMap = typeIt->second;
		size_t idx = 0;
		for (const auto& [name, entry] : entriesMap)
		{
			DOut("     [{}]", idx++);
			entry.LogFileBlock();

			if (auto dataOpt = LoadAssetData<T>(entry))
			{
				dataOpt->LogFileBlock();
			}
		}
	}
#pragma region
}
