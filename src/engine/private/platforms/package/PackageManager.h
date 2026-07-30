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

		[[nodiscard]] const std::map<zzz::package::AssetType, std::vector<zzz::package::PackageEntry>>& GetEntriesByType() const noexcept { return m_EntriesByType; }

	private:
		void Initialize(const Path& path);

		void LogPackageEntriesSummary(const std::filesystem::path& packagePath) const;
		void LogProjectManifestDetails(const std::filesystem::path& packagePath, const zzz::package::PackageEntry& entry) const;
		void LogSceneDetails(const std::filesystem::path& packagePath, const zzz::package::PackageEntry& entry) const;
		void LogViewDetails(const std::filesystem::path& packagePath, const zzz::package::PackageEntry& entry) const;

		std::map<zzz::package::AssetType, std::vector<zzz::package::PackageEntry>> m_EntriesByType;
		zzz::package::PackageHeader m_Header;
	};
}
