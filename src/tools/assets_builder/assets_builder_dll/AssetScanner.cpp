#include "AssetScanner.h"
#include "AssetImporterRegistry.h"

namespace fs = std::filesystem;

namespace zzz::builder
{
	bool ScanAssetsDirectory(const fs::path& assetsDir, const std::function<bool(const ScannedAssetFile&)>& visitor)
	{
		if (!fs::exists(assetsDir) || !fs::is_directory(assetsDir))
			return true;

		for (const auto& entry : fs::recursive_directory_iterator(assetsDir))
		{
			if (!entry.is_regular_file())
				continue;

			fs::path path = entry.path();
			std::string ext = path.extension().string();
			if (ext == ".meta")
				continue;

			ScannedAssetFile file{ path, ext, AssetImporterRegistry::Instance().GetKnownType(ext) };
			if (!visitor(file))
				return false;
		}

		return true;
	}
}
