
#include <fstream>

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

		file.read(reinterpret_cast<char*>(m_Header.magic.data()), 3);
		if (!file.good())
			THROW_RUNTIME("Не удалось прочитать сигнатуру пакета из: {}", packagePath.string());

		if (m_Header.magic != zzz::common::c_GamePackageHeader)
			THROW_RUNTIME("Некорректная сигнатура (magic) заголовка пакета в файле: {}", packagePath.string());

		std::vector<std::byte> versionBuffer(12);
		file.read(reinterpret_cast<char*>(versionBuffer.data()), 12);
		if (!file.good())
			THROW_RUNTIME("Не удалось прочитать версию пакета из: {}", packagePath.string());

		std::size_t offset = 0;
		zzz::common::Serializer serializer;
		auto deSerRes = serializer.Deserialize(versionBuffer, offset, m_Header.version);
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
				DOut("       [{}] GUID: {}", idx, e.GetGuid().ToString());
				DOut("           Offset: {} байт", e.offset);
				DOut("           Size:   {} байт", e.size);

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

	namespace
	{
		zzz::package::BinaryGuid ReadGuid(std::ifstream& file)
		{
			zzz::package::BinaryGuid::RawBytes bytes{};
			file.read(reinterpret_cast<char*>(bytes.data()), 16);
			return zzz::package::BinaryGuid{ bytes };
		}
	}

#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
	void PackageManager::LogProjectManifestDetails([[maybe_unused]] const std::filesystem::path& packagePath, [[maybe_unused]] const zzz::package::PackageEntry& entry) const
	{
		std::ifstream file(packagePath, std::ios::binary);
		if (!file.is_open()) return;

		file.seekg(entry.offset, std::ios::beg);

		zzz::package::BinaryGuid gameScriptGuid = ReadGuid(file);

		DOut("           [ProjectManifest] GameScript GUID: {}", gameScriptGuid.ToString());

		zU32 scenesCount = 0;
		file.read(reinterpret_cast<char*>(&scenesCount), sizeof(scenesCount));
		DOut("           [ProjectManifest] Зарегистрировано сцен: {}", scenesCount);

		for (zU32 i = 0; i < scenesCount; ++i)
		{
			zzz::package::BinaryGuid scGuid = ReadGuid(file);
			DOut("             Scene #{}: {}", i, scGuid.ToString());
		}

		zU32 viewsCount = 0;
		file.read(reinterpret_cast<char*>(&viewsCount), sizeof(viewsCount));
		DOut("           [ProjectManifest] Зарегистрировано видов (views): {}", viewsCount);

		for (zU32 i = 0; i < viewsCount; ++i)
		{
			zzz::package::BinaryGuid vGuid = ReadGuid(file);
			DOut("             View #{}: {}", i, vGuid.ToString());
		}
	}

	void PackageManager::LogSceneDetails([[maybe_unused]] const std::filesystem::path& packagePath, [[maybe_unused]] const zzz::package::PackageEntry& entry) const
	{
		std::ifstream file(packagePath, std::ios::binary);
		if (!file.is_open()) return;

		file.seekg(entry.offset, std::ios::beg);

		zU32 nameLen = 0;
		file.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
		std::string sceneName(nameLen, '\0');
		if (nameLen > 0)
		{
			file.read(sceneName.data(), nameLen);
		}

		zzz::package::BinaryGuid sceneScriptGuid = ReadGuid(file);

		DOut("           [SceneData] Имя сцены: '{}' | Скрипт сцены GUID: {}", sceneName, sceneScriptGuid.ToString());
	}

	void PackageManager::LogViewDetails([[maybe_unused]] const std::filesystem::path& packagePath, [[maybe_unused]] const zzz::package::PackageEntry& entry) const
	{
		std::ifstream file(packagePath, std::ios::binary);
		if (!file.is_open()) return;

		file.seekg(entry.offset, std::ios::beg);

		zU32 viewNameLen = 0;
		file.read(reinterpret_cast<char*>(&viewNameLen), sizeof(viewNameLen));
		std::string viewName(viewNameLen, '\0');
		if (viewNameLen > 0)
		{
			file.read(viewName.data(), viewNameLen);
		}

		zU32 width = 0, height = 0;
		file.read(reinterpret_cast<char*>(&width), sizeof(width));
		file.read(reinterpret_cast<char*>(&height), sizeof(height));
		zzz::common::Size2D<zU32> viewSize(width, height);

		zzz::package::BinaryGuid sceneGuid = ReadGuid(file);

		DOut("           [ViewData] '{}' | Размер: {}x{} | Сцена GUID: {}", viewName, viewSize.width, viewSize.height, sceneGuid.ToString());

		zU32 scriptsCount = 0;
		file.read(reinterpret_cast<char*>(&scriptsCount), sizeof(scriptsCount));
		DOut("           [ViewData] Прикрепленных UI-скриптов: {}", scriptsCount);

		for (zU32 i = 0; i < scriptsCount; ++i)
		{
			zzz::package::BinaryGuid scriptGuid = ReadGuid(file);

			DOut("             UI Script #{}: {}", i, scriptGuid.ToString());
		}
	}
#endif // Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
#pragma endregion
}
