#include "core/utils/Defines.h"

#if defined(Z_ANDROID)

#include "FileSystemAndroid.h"
#include "core/utils/Ensure.h"
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

	[[nodiscard]] std::expected<std::vector<std::byte>, std::string> FileSystemAndroid::ReadBytes(
		eFileLocation location, const std::filesystem::path& relativePath, std::size_t offset, std::size_t size) const noexcept
	{
		if (size == 0)
			return std::vector<std::byte>{};

		if (location == eFileLocation::App)
		{
			auto asset = TryOpenAsset(relativePath, AASSET_MODE_UNKNOWN);
			if (!asset)
				return UNEXPECTED("{}", asset.error());

			const auto totalLength = static_cast<std::size_t>(AAsset_getLength(*asset));
			if (offset + size > totalLength)
			{
				AAsset_close(*asset);
				return UNEXPECTED("Диапазон чтения [{}, {}) выходит за границы ассета '{}' (размер: {})",
					offset, offset + size, relativePath.generic_string(), totalLength);
			}

			if (AAsset_seek(*asset, static_cast<off_t>(offset), SEEK_SET) == -1)
			{
				AAsset_close(*asset);
				return UNEXPECTED("Ошибка позиционирования (seek) в ассете Android: '{}'", relativePath.generic_string());
			}

			std::vector<std::byte> buffer(size);
			const int readBytes = AAsset_read(*asset, buffer.data(), size);
			AAsset_close(*asset);

			if (readBytes < 0 || static_cast<std::size_t>(readBytes) != size)
				return UNEXPECTED("Ошибка чтения диапазона ассета Android: '{}'", relativePath.generic_string());

			return buffer;
		}

		return FileSystemBase::ReadBytes(location, relativePath, offset, size);
	}

	[[nodiscard]] std::expected<std::vector<std::byte>, std::string> FileSystemAndroid::ReadAllBytes(
		eFileLocation location, const std::filesystem::path& relativePath) const noexcept
	{
		if (location == eFileLocation::App)
		{
			auto asset = TryOpenAsset(relativePath, AASSET_MODE_BUFFER);
			if (!asset)
				return UNEXPECTED("{}", asset.error());

			const auto length = static_cast<std::size_t>(AAsset_getLength(*asset));
			if (length == 0)
			{
				AAsset_close(*asset);
				return std::vector<std::byte>{};
			}

			std::vector<std::byte> buffer(length);
			const int readBytes = AAsset_read(*asset, buffer.data(), length);
			AAsset_close(*asset);

			if (readBytes < 0 || static_cast<std::size_t>(readBytes) != length)
				return UNEXPECTED("Ошибка чтения полного ассета Android: '{}'", relativePath.generic_string());

			return buffer;
		}

		return FileSystemBase::ReadAllBytes(location, relativePath);
	}

	std::expected<void, std::string> FileSystemAndroid::WriteAllBytes(
		eFileLocation location, const std::filesystem::path& relativePath, std::span<const std::byte> bytes) noexcept
	{
		if (location == eFileLocation::App)
			return UNEXPECTED("Запись в eFileLocation::App на платформе Android запрещена (read-only пакет APK).");

		return FileSystemBase::WriteAllBytes(location, relativePath, bytes);
	}
}
#endif // defined(Z_ANDROID)
