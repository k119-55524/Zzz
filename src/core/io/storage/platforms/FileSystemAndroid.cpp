#include "core/utils/Defines.h"

#if defined(Z_ANDROID)

#include "FileSystemAndroid.h"
#include "core/utils/Ensure.h"
#include "core/utils/SafeMath.h"
#include "core/utils/macros/MiscMacros.h"
#include <limits>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>

namespace zzz::core
{
	[[nodiscard]] std::expected<AAsset*, std::string> FileSystemAndroid::TryOpenAsset(
		const std::filesystem::path& relativePath, int mode) const noexcept
	{
		auto app = m_NativeData.get();
		if (!app || !app->activity || !app->activity->assetManager)
			return UNEXPECTED("Android AssetManager недоступен.");

		const std::string relStr = relativePath.generic_string();
		AAsset* asset = AAssetManager_open(app->activity->assetManager, relStr.c_str(), mode);
		if (!asset && relStr.starts_with("assets/"))
		{
			// В Android каталог app/src/main/assets является корнем AssetManager,
			// поэтому путь вида "assets/package.dat" может лежать в корне ассетов APK как "package.dat".
			asset = AAssetManager_open(app->activity->assetManager, relStr.c_str() + 7, mode);
		}
		if (!asset)
			return UNEXPECTED("Файл не найден в Android assets: '{}'", relStr);

		return asset;
	}

	[[nodiscard]] bool FileSystemAndroid::FileExists(eFileLocation location, const std::filesystem::path& relativePath) const noexcept
	{
		if (location == eFileLocation::App)
		{
			auto asset = TryOpenAsset(relativePath, AASSET_MODE_UNKNOWN);
			if (asset)
			{
				AAsset_close(*asset);
				return true;
			}
			return false;
		}

		return FileSystemBase::FileExists(location, relativePath);
	}

	[[nodiscard]] std::expected<std::uintmax_t, std::string> FileSystemAndroid::GetFileSize(
		eFileLocation location, const std::filesystem::path& relativePath) const noexcept
	{
		if (location == eFileLocation::App)
		{
			auto asset = TryOpenAsset(relativePath, AASSET_MODE_UNKNOWN);
			if (!asset)
				return UNEXPECTED("{}", asset.error());

			const auto length = AAsset_getLength64(*asset);
			AAsset_close(*asset);
			if (length < 0)
				return UNEXPECTED("Не удалось определить длину Android asset: '{}'", relativePath.string());

			return static_cast<std::uintmax_t>(length);
		}

		return FileSystemBase::GetFileSize(location, relativePath);
	}
}

#endif // defined(Z_ANDROID)
