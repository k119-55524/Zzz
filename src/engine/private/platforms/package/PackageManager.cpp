#include <fstream>
#include <common/io/package/ViewData.h>
#include <common/io/package/SceneData.h>
#include <common/io/package/ProjectManifestData.h>

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

		std::filesystem::path packagePath = *execDir / c_GamePackageFileName;

		if (!std::filesystem::exists(packagePath))
			THROW_RUNTIME("Файл пакета не существует: {}", packagePath.string());

		std::ifstream file(packagePath, std::ios::binary);
		if (!file.is_open())
			THROW_RUNTIME("Не удалось открыть файл пакета: {}", packagePath.string());

		const auto fileSize = std::filesystem::file_size(packagePath);
		std::vector<std::byte> fileBuffer(fileSize);
		file.read(reinterpret_cast<char*>(fileBuffer.data()), fileSize);
		if (!file.good())
			THROW_RUNTIME("Не удалось прочитать файл пакета из: {}", packagePath.string());

		std::size_t offset = 0;
		Serializer serializer;

		auto headerRes = serializer.Deserialize(fileBuffer, offset, m_Header);
		if (!headerRes)
			THROW_RUNTIME("Ошибка десериализации заголовка пакета {}: {}", packagePath.string(), headerRes.error());

		auto validRes = m_Header.Validate();
		if (!validRes)
			THROW_RUNTIME("Некорректный заголовок в файле {}: {}", packagePath.string(), validRes.error());

		for (zU32 i = 0; i < m_Header.GetEntryCount(); ++i)
		{
			PackageEntry entry{};
			auto entryRes = serializer.Deserialize(fileBuffer, offset, entry);
			if (!entryRes)
				THROW_RUNTIME("Ошибка десериализации записи пакета #{} в файле {}: {}", i, packagePath.string(), entryRes.error());

			m_EntriesByType[static_cast<ePackage>(entry.assetType)].push_back(entry);
		}

		LogPackageEntriesSummary(packagePath);
	}

#pragma region Logging
	void PackageManager::LogPackageEntriesSummary([[maybe_unused]] const std::filesystem::path& packagePath) const
	{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
		DOut("[PackageManager] Инициализация пакета: {}", packagePath.string());
		m_Header.LogFileBlock();

		for (const auto& [type, entries] : m_EntriesByType)
		{
			DOut("  -> AssetType: {}: {} штук", EnumToString::ToString(type), entries.size());

			for (size_t idx = 0; idx < entries.size(); ++idx)
			{
				const auto& entry = entries[idx];
				DOut("     [{}]", idx);
				entry.LogFileBlock();
				LogAssetDetails(packagePath, entry);
			}
		}
#endif
	}

#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
	template <typename T> requires std::derived_from<T, ISerializable> && std::derived_from<T, IFileBlockLoggable>
	static void LogBlockData(const std::filesystem::path& packagePath, const PackageEntry& entry)
	{
		std::ifstream file(packagePath, std::ios::binary);
		if (!file.is_open()) return;

		file.seekg(entry.offset, std::ios::beg);

		std::vector<std::byte> buffer(entry.size);
		file.read(reinterpret_cast<char*>(buffer.data()), entry.size);
		if (!file.good()) return;

		std::size_t offset = 0;
		Serializer serializer;
		T data{};

		if (serializer.Deserialize(buffer, offset, data))
		{
			data.LogFileBlock();
		}
	}

	void PackageManager::LogAssetDetails([[maybe_unused]] const std::filesystem::path& packagePath, [[maybe_unused]] const PackageEntry& entry) const
	{
		switch (static_cast<ePackage>(entry.assetType))
		{
			case ePackage::ProjectManifest: LogBlockData<ProjectManifestData>(packagePath, entry); break;
			case ePackage::Scene:           LogBlockData<SceneData>(packagePath, entry); break;
			case ePackage::View:            LogBlockData<ViewData>(packagePath, entry); break;
			default: break;
		}
	}
#endif // Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
#pragma region
}
