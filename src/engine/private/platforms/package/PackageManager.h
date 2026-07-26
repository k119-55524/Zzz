#pragma once

#include <string>
#include <vector>
#include <expected>
#include <filesystem>
#include <unordered_map>
#include <common/package_format.h>

namespace zzz::engine
{
	class PackageManager final
	{
	public:
		PackageManager() = default;
		~PackageManager() = default;

		[[nodiscard]] std::expected<void, std::string> Initialize(const std::filesystem::path& packagePath);
		[[nodiscard]] bool HasAsset(const std::string& guid) const noexcept;
		[[nodiscard]] std::expected<std::vector<std::byte>, std::string> ReadAssetData(const std::string& guid) const;

		[[nodiscard]] const std::unordered_map<std::string, zzz::package::PackageEntry>& GetEntries() const noexcept { return m_Entries; }

	private:
		std::filesystem::path m_PackagePath;
		package::PackageHeader m_Header{};
		std::unordered_map<std::string, zzz::package::PackageEntry> m_Entries;
		bool m_IsInitialized{ false };
	};
}
