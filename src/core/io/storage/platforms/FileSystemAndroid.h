#pragma once

#include "core/utils/Defines.h"

#if defined(Z_ANDROID)

#include "core/io/storage/FileSystemBase.h"

struct AAsset;

namespace zzz::core
{
	/**
	 * @class FileSystemAndroid
	 * @brief Специализация файловой системы для платформы Android.
	 *
	 * @details eFileLocation::App читается напрямую из закрытого APK через NDK AAssetManager.
	 *          Все пользовательские данные (User, Saves, Cache, Logs) делегируются в FileSystemBase.
	 */
	class FileSystemAndroid final : public FileSystemBase
	{
	public:
		using FileSystemBase::FileSystemBase;
		~FileSystemAndroid() = default;

		[[nodiscard]] bool FileExists(eFileLocation location, const std::filesystem::path& relativePath) const noexcept override;
		[[nodiscard]] std::expected<std::uintmax_t, std::string> GetFileSize(
			eFileLocation location, const std::filesystem::path& relativePath) const noexcept override;

	private:
		[[nodiscard]] std::expected<AAsset*, std::string> TryOpenAsset(
			const std::filesystem::path& relativePath, int mode = 0) const noexcept;
	};
}

#endif // defined(Z_ANDROID)
