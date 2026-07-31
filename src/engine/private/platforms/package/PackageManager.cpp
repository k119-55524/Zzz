
#include <fstream>
#include <common/package/ViewData.h>
#include <common/package/SceneData.h>
#include <common/package/ProjectManifestData.h>

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
		DOut("[PackageManager] Пакет инициализирован: {} | Версия: {} | Записей: {}",
			packagePath.string(), m_Header.GetVersion().ToString(), m_Header.GetEntryCount());

		for (const auto& [type, entries] : m_EntriesByType)
		{
			const char* typeName = "Unknown";
			switch (type)
			{
				case ePackage::ProjectManifest:	typeName = "ProjectManifest";	break;
				case ePackage::Scene:				typeName = "Scene";				break;
				case ePackage::View:				typeName = "View";				break;
				case ePackage::BinaryAsset:		typeName = "BinaryAsset";		break;
			}

			DOut("  -> AssetType: {}: {} штук", typeName, entries.size());

			for (size_t idx = 0; idx < entries.size(); ++idx)
			{
				const auto& e = entries[idx];
				DOut("       [{}] GUID: {}", idx, e.GetGuid().ToString());
				DOut("           Offset: {} байт", e.offset);
				DOut("           Size:   {} байт", e.size);

				if (type == ePackage::ProjectManifest)
				{
					LogProjectManifestDetails(packagePath, e);
				}
				else if (type == ePackage::Scene)
				{
					LogSceneDetails(packagePath, e);
				}
				else if (type == ePackage::View)
				{
					LogViewDetails(packagePath, e);
				}
			}
		}
#endif
	}

#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
	void PackageManager::LogProjectManifestDetails([[maybe_unused]] const std::filesystem::path& packagePath, [[maybe_unused]] const PackageEntry& entry) const
	{
		std::ifstream file(packagePath, std::ios::binary);
		if (!file.is_open()) return;

		file.seekg(entry.offset, std::ios::beg);

		std::vector<std::byte> buffer(entry.size);
		file.read(reinterpret_cast<char*>(buffer.data()), entry.size);
		if (!file.good()) return;

		std::size_t offset = 0;
		Serializer serializer;
		zzz::core::ProjectManifestData manifestData;

		auto deSerRes = serializer.Deserialize(buffer, offset, manifestData);
		if (!deSerRes) return;

		DOut("           [ProjectManifest] GameScript GUID: {}", manifestData.gameScriptGuid.ToString());
		DOut("           [ProjectManifest] Зарегистрировано сцен: {}", manifestData.sceneGuids.size());

		for (zU32 i = 0; i < manifestData.sceneGuids.size(); ++i)
		{
			DOut("             Scene #{}: {}", i, manifestData.sceneGuids[i].ToString());
		}

		DOut("           [ProjectManifest] Зарегистрировано видов (views): {}", manifestData.viewGuids.size());

		for (zU32 i = 0; i < manifestData.viewGuids.size(); ++i)
		{
			DOut("             View #{}: {}", i, manifestData.viewGuids[i].ToString());
		}
	}

	void PackageManager::LogSceneDetails([[maybe_unused]] const std::filesystem::path& packagePath, [[maybe_unused]] const PackageEntry& entry) const
	{
		std::ifstream file(packagePath, std::ios::binary);
		if (!file.is_open()) return;

		file.seekg(entry.offset, std::ios::beg);

		std::vector<std::byte> buffer(entry.size);
		file.read(reinterpret_cast<char*>(buffer.data()), entry.size);
		if (!file.good()) return;

		std::size_t offset = 0;
		Serializer serializer;
		SceneData sceneData;

		auto deSerRes = serializer.Deserialize(buffer, offset, sceneData);
		if (!deSerRes) return;

		DOut("           [SceneData] Имя сцены: '{}' | Скрипт сцены GUID: {}", sceneData.name, sceneData.sceneScriptGuid.ToString());
	}

	void PackageManager::LogViewDetails([[maybe_unused]] const std::filesystem::path& packagePath, [[maybe_unused]] const PackageEntry& entry) const
	{
		std::ifstream file(packagePath, std::ios::binary);
		if (!file.is_open()) return;

		file.seekg(entry.offset, std::ios::beg);

		std::vector<std::byte> buffer(entry.size);
		file.read(reinterpret_cast<char*>(buffer.data()), entry.size);
		if (!file.good()) return;

		std::size_t offset = 0;
		Serializer serializer;
		ViewData viewData;

		auto deSerRes = serializer.Deserialize(buffer, offset, viewData);
		if (!deSerRes) return;

		DOut("           [ViewData] '{}' | Размер: {}x{} | Сцена GUID: {}", viewData.name, viewData.size.width, viewData.size.height, viewData.sceneGuid.ToString());
		DOut("           [ViewData] Прикрепленных UI-скриптов: {}", viewData.uiScriptGuids.size());

		for (zU32 i = 0; i < viewData.uiScriptGuids.size(); ++i)
		{
			DOut("             UI Script #{}: {}", i, viewData.uiScriptGuids[i].ToString());
		}
	}
#endif // Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
#pragma endregion
}
