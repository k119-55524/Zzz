#pragma once

#include <string>
#include <filesystem>

using namespace zzz;

namespace zzz::io
{
	class Path final
	{
	public:
		Path() = delete;
		Path(std::string_view appName, std::shared_ptr<void> platformData);

		[[nodiscard]] static bool IsValidDirectoryName(std::string_view name);

		[[nodiscard]] std::expected<std::filesystem::path, std::string> GetExecutableDirectory();
		inline std::filesystem::path GetUserDataDirectory() noexcept { return m_UserDataDirectory; };

	private:
		std::string_view m_AppName;
		std::shared_ptr<void>  m_PlatformData;
		std::filesystem::path m_UserDataDirectory;

		[[nodiscard]] std::expected<std::filesystem::path, std::string> ResolveUserDataDirectory();

#if defined(Z_MACOS) || defined(Z_IOS)
		[[nodiscard]] std::expected<std::filesystem::path, std::string> GetAppleUserDataDirectory();
#endif
	};
}
