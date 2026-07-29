#include <fstream>
#include <cstring>

#include "PackageManager.h"

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

		std::filesystem::path packagePath = *execDir / zzz::common::c_GamePackageFileName;

		if (!std::filesystem::exists(packagePath))
			THROW_RUNTIME("Файл пакета не существует: {}", packagePath.string());

		std::ifstream file(packagePath, std::ios::binary);
		if (!file.is_open())
			THROW_RUNTIME("Не удалось открыть файл пакета: {}", packagePath.string());

		m_Header = {};

		// 1. Читаем magic (3 байта)
		file.read(reinterpret_cast<char*>(m_Header.magic.data()), 3);
		if (!file.good())
			THROW_RUNTIME("Не удалось прочитать сигнатуру пакета из: {}", packagePath.string());

		if (m_Header.magic != zzz::common::c_GamePackageHeader)
			THROW_RUNTIME("Некорректная сигнатура (magic) заголовка пакета в файле: {}", packagePath.string());

		// 2. Читаем 12 байт версии через движковый Serializer::DeSerialize
		std::vector<std::byte> versionBuffer(12);
		file.read(reinterpret_cast<char*>(versionBuffer.data()), 12);
		if (!file.good())
			THROW_RUNTIME("Не удалось прочитать версию пакета из: {}", packagePath.string());

		std::size_t offset = 0;
		zzz::common::Serializer serializer;
		auto deSerRes = serializer.DeSerialize(versionBuffer, offset, m_Header.version);
		if (!deSerRes)
			THROW_RUNTIME("Ошибка десериализации версии пакета в файле {}: {}", packagePath.string(), deSerRes.error());

		// 3. Читаем entryCount (4 байта)
		file.read(reinterpret_cast<char*>(&m_Header.entryCount), sizeof(m_Header.entryCount));
		if (!file.good())
			THROW_RUNTIME("Не удалось прочитать количество элементов пакета из: {}", packagePath.string());

		for (zU32 i = 0; i < m_Header.entryCount; ++i)
		{
			zzz::package::PackageEntry entry{};
			file.read(reinterpret_cast<char*>(&entry), sizeof(zzz::package::PackageEntry));
			if (!file.good())
				THROW_RUNTIME("Поврежденная таблица записей пакета в файле: {}", packagePath.string());

			m_EntriesByType[static_cast<zzz::package::AssetType>(entry.assetType)].push_back(entry);
		}

		LogPackageEntriesSummary(packagePath);
	}

#pragma region Logging
	void PackageManager::LogPackageEntriesSummary([[maybe_unused]] const std::filesystem::path& packagePath) const
	{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
		DOut("[PackageManager] Пакет инициализирован: {} | Версия: {} | Записей: {}",
			packagePath.string(), m_Header.version.ToString(), m_Header.entryCount);

		for (const auto& [type, entries] : m_EntriesByType)
		{
			const char* typeName = "Unknown";
			switch (type)
			{
				case zzz::package::AssetType::ProjectManifest:	typeName = "ProjectManifest";	break;
				case zzz::package::AssetType::Scene:			typeName = "Scene";				break;
				case zzz::package::AssetType::View:				typeName = "View";				break;
				case zzz::package::AssetType::BinaryAsset:		typeName = "BinaryAsset";		break;
			}

			DOut("  -> AssetType: {}: {} штук", typeName, entries.size());

			for (size_t idx = 0; idx < entries.size(); ++idx)
			{
				const auto& e = entries[idx];
				DOut("       [{}] GUID: {:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x} | Offset: {} | Size: {} байт",
					idx,
					e.guid[0], e.guid[1], e.guid[2], e.guid[3],
					e.guid[4], e.guid[5],
					e.guid[6], e.guid[7],
					e.guid[8], e.guid[9],
					e.guid[10], e.guid[11], e.guid[12], e.guid[13], e.guid[14], e.guid[15],
					e.offset, e.size);

				if (type == zzz::package::AssetType::ProjectManifest)
				{
					LogProjectManifestDetails(packagePath, e);
				}
				else if (type == zzz::package::AssetType::Scene)
				{
					LogSceneDetails(packagePath, e);
				}
				else if (type == zzz::package::AssetType::View)
				{
					LogViewDetails(packagePath, e);
				}
			}
		}
#endif
	}

	void PackageManager::LogProjectManifestDetails([[maybe_unused]] const std::filesystem::path& packagePath, [[maybe_unused]] const zzz::package::PackageEntry& entry) const
	{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
		std::ifstream file(packagePath, std::ios::binary);
		if (!file.is_open()) return;

		file.seekg(entry.offset, std::ios::beg);

		// 1. GameScript GUID (16 байт)
		zzz::package::BinaryGuid gameScriptGuid = {0};
		file.read(reinterpret_cast<char*>(gameScriptGuid.data()), 16);

		DOut("           [ProjectManifest] GameScript GUID: {:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
			gameScriptGuid[0], gameScriptGuid[1], gameScriptGuid[2], gameScriptGuid[3],
			gameScriptGuid[4], gameScriptGuid[5],
			gameScriptGuid[6], gameScriptGuid[7],
			gameScriptGuid[8], gameScriptGuid[9],
			gameScriptGuid[10], gameScriptGuid[11], gameScriptGuid[12], gameScriptGuid[13], gameScriptGuid[14], gameScriptGuid[15]);

		// 2. Scenes GUIDs
		zU32 scenesCount = 0;
		file.read(reinterpret_cast<char*>(&scenesCount), sizeof(scenesCount));
		DOut("           [ProjectManifest] Зарегистрировано сцен: {}", scenesCount);

		for (zU32 i = 0; i < scenesCount; ++i)
		{
			zzz::package::BinaryGuid scGuid = {0};
			file.read(reinterpret_cast<char*>(scGuid.data()), 16);
			DOut("             Scene #{}: {:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
				i, scGuid[0], scGuid[1], scGuid[2], scGuid[3], scGuid[4], scGuid[5], scGuid[6], scGuid[7],
				scGuid[8], scGuid[9], scGuid[10], scGuid[11], scGuid[12], scGuid[13], scGuid[14], scGuid[15]);
		}

		// 3. Views GUIDs
		zU32 viewsCount = 0;
		file.read(reinterpret_cast<char*>(&viewsCount), sizeof(viewsCount));
		DOut("           [ProjectManifest] Зарегистрировано видов (views): {}", viewsCount);

		for (zU32 i = 0; i < viewsCount; ++i)
		{
			zzz::package::BinaryGuid vGuid = {0};
			file.read(reinterpret_cast<char*>(vGuid.data()), 16);
			DOut("             View #{}: {:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
				i, vGuid[0], vGuid[1], vGuid[2], vGuid[3], vGuid[4], vGuid[5], vGuid[6], vGuid[7],
				vGuid[8], vGuid[9], vGuid[10], vGuid[11], vGuid[12], vGuid[13], vGuid[14], vGuid[15]);
		}
#endif
	}

	void PackageManager::LogSceneDetails([[maybe_unused]] const std::filesystem::path& packagePath, [[maybe_unused]] const zzz::package::PackageEntry& entry) const
	{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
		std::ifstream file(packagePath, std::ios::binary);
		if (!file.is_open()) return;

		file.seekg(entry.offset, std::ios::beg);

		// 1. Scene script GUID (16 байт)
		zzz::package::BinaryGuid sceneScriptGuid = {0};
		file.read(reinterpret_cast<char*>(sceneScriptGuid.data()), 16);

		DOut("           [SceneData] Скрипт сцены GUID: {:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
			sceneScriptGuid[0], sceneScriptGuid[1], sceneScriptGuid[2], sceneScriptGuid[3],
			sceneScriptGuid[4], sceneScriptGuid[5],
			sceneScriptGuid[6], sceneScriptGuid[7],
			sceneScriptGuid[8], sceneScriptGuid[9],
			sceneScriptGuid[10], sceneScriptGuid[11], sceneScriptGuid[12], sceneScriptGuid[13], sceneScriptGuid[14], sceneScriptGuid[15]);
#endif
	}

	void PackageManager::LogViewDetails([[maybe_unused]] const std::filesystem::path& packagePath, [[maybe_unused]] const zzz::package::PackageEntry& entry) const
	{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
		std::ifstream file(packagePath, std::ios::binary);
		if (!file.is_open()) return;

		file.seekg(entry.offset, std::ios::beg);

		// 1. Scene GUID (16 байт)
		zzz::package::BinaryGuid sceneGuid = {0};
		file.read(reinterpret_cast<char*>(sceneGuid.data()), 16);

		DOut("           [ViewData] Сцена GUID: {:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
			sceneGuid[0], sceneGuid[1], sceneGuid[2], sceneGuid[3],
			sceneGuid[4], sceneGuid[5],
			sceneGuid[6], sceneGuid[7],
			sceneGuid[8], sceneGuid[9],
			sceneGuid[10], sceneGuid[11], sceneGuid[12], sceneGuid[13], sceneGuid[14], sceneGuid[15]);

		// 2. UI Scripts count (4 байта)
		zU32 scriptsCount = 0;
		file.read(reinterpret_cast<char*>(&scriptsCount), sizeof(scriptsCount));
		DOut("           [ViewData] Прикрепленных UI-скриптов: {}", scriptsCount);

		for (zU32 i = 0; i < scriptsCount; ++i)
		{
			zzz::package::BinaryGuid scriptGuid = {0};
			file.read(reinterpret_cast<char*>(scriptGuid.data()), 16);

			DOut("             UI Script #{}: {:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
				i,
				scriptGuid[0], scriptGuid[1], scriptGuid[2], scriptGuid[3],
				scriptGuid[4], scriptGuid[5],
				scriptGuid[6], scriptGuid[7],
				scriptGuid[8], scriptGuid[9],
				scriptGuid[10], scriptGuid[11], scriptGuid[12], scriptGuid[13], scriptGuid[14], scriptGuid[15]);
		}

		// 3. Elements count (4 байта)
		zU32 elementsCount = 0;
		file.read(reinterpret_cast<char*>(&elementsCount), sizeof(elementsCount));
		DOut("           [ViewData] GUI Элементов экрана: {}", elementsCount);
#endif
	}
#pragma endregion

	//bool PackageManager::HasAsset(const zzz::package::BinaryGuid& guid) const noexcept
	//{
	//	if (!m_IsInitialized)
	//		return false;

	//	return m_Entries.contains(guid);
	//}

	//std::expected<std::vector<std::byte>, std::string> PackageManager::ReadAssetData(const zzz::package::BinaryGuid& guid) const
	//{
	//	if (!m_IsInitialized)
	//	{
	//		return std::unexpected("PackageManager is not initialized");
	//	}

	//	auto it = m_Entries.find(guid);
	//	if (it == m_Entries.end())
	//	{
	//		return std::unexpected("Asset GUID not found in package");
	//	}

	//	const auto& entry = it->second;
	//	std::ifstream file(m_PackagePath, std::ios::binary);
	//	if (!file.is_open())
	//	{
	//		return std::unexpected("Failed to open package file: " + m_PackagePath.string());
	//	}

	//	file.seekg(entry.offset, std::ios::beg);
	//	std::vector<std::byte> buffer(entry.size);
	//	file.read(reinterpret_cast<char*>(buffer.data()), entry.size);

	//	if (!file.good() && file.gcount() != static_cast<std::streamsize>(entry.size))
	//	{
	//		return std::unexpected("Failed to read asset data block for GUID");
	//	}

	//	return buffer;
	//}
}
