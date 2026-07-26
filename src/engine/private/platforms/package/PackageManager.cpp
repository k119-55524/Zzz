
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
		file.read(reinterpret_cast<char*>(&m_Header), sizeof(zzz::package::PackageHeader));
		if (!file.good())
			THROW_RUNTIME("Не удалось прочитать заголовок пакета из: {}", packagePath.string());

		if (m_Header.magic != zzz::common::c_GamePackageHeader)
			THROW_RUNTIME("Некорректная сигнатура (magic) заголовка пакета в файле: {}", packagePath.string());

		for (zU32 i = 0; i < m_Header.entryCount; ++i)
		{
			zzz::package::PackageEntry entry{};
			file.read(reinterpret_cast<char*>(&entry), sizeof(zzz::package::PackageEntry));
			if (!file.good())
				THROW_RUNTIME("Поврежденная таблица записей пакета в файле: {}", packagePath.string());

			std::string guidStr(entry.guid, strnlen(entry.guid, sizeof(entry.guid)));
			m_Entries[guidStr] = entry;
		}
	}

	//bool PackageManager::HasAsset(const std::string& guid) const noexcept
	//{
	//	if (!m_IsInitialized)
	//		return false;

	//	return m_Entries.contains(guid);
	//}

	//std::expected<std::vector<std::byte>, std::string> PackageManager::ReadAssetData(const std::string& guid) const
	//{
	//	if (!m_IsInitialized)
	//	{
	//		return std::unexpected("PackageManager is not initialized");
	//	}

	//	auto it = m_Entries.find(guid);
	//	if (it == m_Entries.end())
	//	{
	//		return std::unexpected("Asset GUID not found in package: " + guid);
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
	//		return std::unexpected("Failed to read asset data block for GUID: " + guid);
	//	}

	//	return buffer;
	//}
}
