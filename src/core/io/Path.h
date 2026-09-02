#pragma once

#include "core/utils/Defines.h"

#include <memory>
#include <string>
#include <expected>
#include <filesystem>
#include <string_view>

#include "core/utils/NativeAppData.h"
#include "core/enums/eUserDirectoryKind.h"
#include "core/constants/ConfigConstants.h"
#include "core/enums/eAssetDirectoryKind.h"
#include "core/constants/PackageConstants.h"

namespace zzz::core
{
	class FileSystemBase;

	class Path final
	{
		friend class FileSystemBase;

	public:
		Path() = delete;
		explicit Path(std::shared_ptr<NativeAppData> nativeData = nullptr);

		[[nodiscard]] inline const std::filesystem::path& GetExecutableDirectory() const noexcept { return m_ExecutableDirectory; }
		[[nodiscard]] inline const std::filesystem::path& GetUserDataDirectory() const noexcept { return m_UserDataDirectory; }

		[[nodiscard]] inline std::filesystem::path GetPackageDatPath() const { return m_ExecutableDirectory / c_GamePackageRelativePath; }
		[[nodiscard]] inline std::filesystem::path GetUserDatPath() const { return m_UserDataDirectory / c_ConfigFileName; }
		[[nodiscard]] inline std::filesystem::path GetDataDatPath() const { return m_ExecutableDirectory / c_DataPackageRelativePath; }

		[[nodiscard]] std::filesystem::path GetDirectory(eUserDirectoryKind kind) const;
		[[nodiscard]] std::filesystem::path GetDirectory(eAssetDirectoryKind kind) const;

		[[nodiscard]] static bool IsValidDirectoryName(std::string_view name) noexcept;

	private:
		[[nodiscard]] static std::expected<std::filesystem::path, std::string> ResolveExecutableDirectory() noexcept;
		[[nodiscard]] std::expected<void, std::string> InitializeUserData(std::string_view companyName, std::string_view appName);
		[[nodiscard]] std::expected<std::filesystem::path, std::string> ResolveUserDataDirectory(std::string_view companyName, std::string_view appName);

		std::shared_ptr<NativeAppData> m_NativeData;
		std::filesystem::path m_ExecutableDirectory;
		std::filesystem::path m_UserDataDirectory;

#if defined(Z_APPLE)
		[[nodiscard]] std::expected<std::filesystem::path, std::string> GetAppleUserDataDirectory();
#endif
	};
}
