#include "PackageManager.h"
#include <fstream>
#include <cstring>

namespace zzz::engine
{
	std::expected<void, std::string> PackageManager::Initialize(const std::filesystem::path& packagePath)
	{
		m_PackagePath = packagePath;
		m_Entries.clear();
		m_IsInitialized = false;

		if (!std::filesystem::exists(m_PackagePath))
		{
			return std::unexpected("Package file does not exist: " + m_PackagePath.string());
		}

		std::ifstream file(m_PackagePath, std::ios::binary);
		if (!file.is_open())
		{
			return std::unexpected("Failed to open package file: " + m_PackagePath.string());
		}

		// Reading header
		file.read(reinterpret_cast<char*>(&m_Header), sizeof(zzz::package::PackageHeader));
		if (!file.good())
		{
			return std::unexpected("Failed to read package header from: " + m_PackagePath.string());
		}

		// Verify header magic
		if (m_Header.magic != zzz::common::c_GamePackageHeader)
		{
			return std::unexpected("Invalid package header magic in file: " + m_PackagePath.string());
		}

		// Reading entry index table
		for (zU32 i = 0; i < m_Header.entryCount; ++i)
		{
			zzz::package::PackageEntry entry{};
			file.read(reinterpret_cast<char*>(&entry), sizeof(zzz::package::PackageEntry));
			if (!file.good())
			{
				return std::unexpected("Corrupted package entry table in file: " + m_PackagePath.string());
			}

			std::string guidStr(entry.guid, strnlen(entry.guid, sizeof(entry.guid)));
			m_Entries[guidStr] = entry;
		}

		m_IsInitialized = true;
		return {};
	}

	bool PackageManager::HasAsset(const std::string& guid) const noexcept
	{
		if (!m_IsInitialized)
			return false;

		return m_Entries.contains(guid);
	}

	std::expected<std::vector<std::byte>, std::string> PackageManager::ReadAssetData(const std::string& guid) const
	{
		if (!m_IsInitialized)
		{
			return std::unexpected("PackageManager is not initialized");
		}

		auto it = m_Entries.find(guid);
		if (it == m_Entries.end())
		{
			return std::unexpected("Asset GUID not found in package: " + guid);
		}

		const auto& entry = it->second;
		std::ifstream file(m_PackagePath, std::ios::binary);
		if (!file.is_open())
		{
			return std::unexpected("Failed to open package file: " + m_PackagePath.string());
		}

		file.seekg(entry.offset, std::ios::beg);
		std::vector<std::byte> buffer(entry.size);
		file.read(reinterpret_cast<char*>(buffer.data()), entry.size);

		if (!file.good() && file.gcount() != static_cast<std::streamsize>(entry.size))
		{
			return std::unexpected("Failed to read asset data block for GUID: " + guid);
		}

		return buffer;
	}
}
