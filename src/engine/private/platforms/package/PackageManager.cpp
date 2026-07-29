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

		package::PackageHeader m_Header{};

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
				case zzz::package::AssetType::Script:			typeName = "Script";			break;
				case zzz::package::AssetType::BinaryAsset:		typeName = "BinaryAsset";		break;
			}

			DOut("  -> Тип {} ({}): {} элементов.", static_cast<uint32_t>(type), typeName, entries.size());

			for (const auto& e : entries)
				DOut("		Offset: {}, Size: {} байт", e.offset, e.size);
		}
#endif // Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
	}

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
