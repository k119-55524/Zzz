#pragma once

#include <string>
#include <vector>
#include <expected>
#include <filesystem>
#include <map>
#include <common/package_format.h>

#include "../core/io/Path.h"

using namespace zzz::io;

namespace zzz::engine
{
	class PackageManager final
	{
	public:
		PackageManager() = delete;
		PackageManager(const Path& path);
		~PackageManager() = default;

		//[[nodiscard]] std::expected<void, std::string> Initialize(const std::filesystem::path& packagePath);
		//[[nodiscard]] bool HasAsset(const zzz::package::BinaryGuid& guid) const noexcept;
		//[[nodiscard]] std::expected<std::vector<std::byte>, std::string> ReadAssetData(const zzz::package::BinaryGuid& guid) const;

		//[[nodiscard]] const std::map<zzz::package::BinaryGuid, zzz::package::PackageEntry>& GetEntries() const noexcept { return m_Entries; }

	private:
		void Initialize(const Path& path);

		std::map<zzz::package::BinaryGuid, zzz::package::PackageEntry> m_Entries;
	};
}
