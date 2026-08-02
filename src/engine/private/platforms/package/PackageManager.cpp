#include <fstream>
#include <core/IO/package/ViewData.h>
#include <core/IO/package/SceneData.h>
#include <core/IO/package/PrefabData.h>
#include <core/IO/package/ProjectManifestData.h>

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

			auto type = static_cast<ePackage>(entry.GetAssetType());
			m_EntriesByName[type][entry.GetName()] = entry;
			m_EntriesByGuid[type][entry.GetGuid()] = entry;
		}

		auto appViewIt = m_EntriesByName.find(ePackage::AppView);
		if (appViewIt == m_EntriesByName.end() || appViewIt->second.empty())
		{
			THROW_RUNTIME("Ошибка пакета {}: Обязательный ресурс AppViewData отсутствует.", m_PackagePath.string());
		}
		if (appViewIt->second.size() > 1)
		{
			THROW_RUNTIME("Ошибка пакета {}: Ресурс AppViewData не уникален (найдено {} штук).", m_PackagePath.string(), appViewIt->second.size());
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

	std::optional<AppViewData> PackageManager::GetAppViewData() const
	{
		auto typeIt = m_EntriesByName.find(ePackage::AppView);
		if (typeIt == m_EntriesByName.end() || typeIt->second.empty())
			return std::nullopt;

		const auto& entry = typeIt->second.begin()->second;
		return LoadAssetData<AppViewData>(entry);
	}

	template <typename T> requires std::derived_from<T, ISerializable>
	[[nodiscard]] std::optional<T> PackageManager::LoadAssetData(const PackageEntry& entry) const
	{
		if (m_PackagePath.empty())
			return std::nullopt;

		std::ifstream file(m_PackagePath, std::ios::binary);
		if (!file.is_open())
			return std::nullopt;

		file.seekg(entry.GetOffset(), std::ios::beg);
		std::vector<std::byte> buffer(entry.GetSize());
		file.read(reinterpret_cast<char*>(buffer.data()), entry.GetSize());
		if (!file.good())
			return std::nullopt;

		std::size_t offset = 0;
		Serializer serializer;
		T data{};
		if (serializer.Deserialize(buffer, offset, data))
		{
			if constexpr (std::is_same_v<T, ViewData>)
			{
				data.SetName(entry.GetName());
			}
			return data;
		}

		return std::nullopt;
	}

#pragma region Logging
	void PackageManager::LogPackageEntriesSummary() const
	{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
		// Закомментируй тот тип ресурса, который не хочешь логировать
		LogEntriesSummaryForType<ProjectManifestData>(ePackage::ProjectManifest);
		LogEntriesSummaryForType<AppViewData>(ePackage::AppView);
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
#pragma endregion
}
