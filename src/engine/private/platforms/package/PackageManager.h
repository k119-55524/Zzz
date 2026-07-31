#pragma once

#include <map>
#include <vector>
#include <filesystem>
#include <common/enums/ePackage.h>
#include <common/io/package/PackageHeader.h>
#include <common/io/package/PackageEntry.h>

#include "../core/io/Path.h"

using namespace zzz::io;
using namespace zzz::core;
using namespace zzz::common;

namespace zzz::engine
{
	class PackageManager final
	{
	public:
		PackageManager() = delete;
		PackageManager(const Path& path);
		~PackageManager() = default;

		[[nodiscard]] const std::map<ePackage, std::vector<PackageEntry>>& GetEntriesByType() const noexcept { return m_EntriesByType; }

	private:
		void Initialize(const Path& path);
		void LogPackageEntriesSummary(const std::filesystem::path& packagePath) const;

		PackageHeader m_Header;
		std::map<ePackage, std::vector<PackageEntry>> m_EntriesByType;

#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
		void LogAssetDetails(const std::filesystem::path& packagePath, const PackageEntry& entry) const;
#endif // Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
	};
}
