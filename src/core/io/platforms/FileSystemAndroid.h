#pragma once

#include "core/utils/Defines.h"

#if defined(Z_ANDROID)

#include "core/io/FileSystemBase.h"

struct AAsset;

namespace zzz::core
{
	/**
	 * @class FileSystemAndroid
	 * @brief Специализация файловой системы для платформы Android.
	 *
	 * @details eFileLocation::App читается напрямую из закрытого APK через NDK AAssetManager.
	 *          Запись в eFileLocation::App запрещена.
	 *          Все пользовательские данные (User, Saves, Cache, Logs) делегируются в FileSystemBase.
	 */
	class FileSystemAndroid final : public FileSystemBase
	{
	public:
		using FileSystemBase::FileSystemBase;
		~FileSystemAndroid() = default;

		[[nodiscard]] bool FileExists(eFileLocation location, std::string_view relativePath) const noexcept;

		[[nodiscard]] std::expected<std::vector<std::byte>, std::string> ReadBytes(
			eFileLocation location, std::string_view relativePath, std::size_t offset, std::size_t size) const noexcept;

		[[nodiscard]] std::expected<std::vector<std::byte>, std::string> ReadAllBytes(
			eFileLocation location, std::string_view relativePath) const noexcept;

		std::expected<void, std::string> WriteAllBytes(
			eFileLocation location, std::string_view relativePath, std::span<const std::byte> bytes) noexcept;

	private:
		[[nodiscard]] std::expected<AAsset*, std::string> TryOpenAsset(
			std::string_view relativePath, int mode = 0) const noexcept;
	};
}

#endif // defined(Z_ANDROID)
