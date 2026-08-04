#pragma once

#include <string>
#include <string_view>
#include <expected>
#include <filesystem>
#include <memory>

#include <engine/NativeAppData.h>

namespace zzz::core
{
	class Path final
	{
	public:
		Path() = delete;
		Path(std::string_view appName, std::shared_ptr<zzz::engine::NativeAppData> nativeData);

		inline std::string_view GetAppName() const noexcept { return m_AppName; }

		[[nodiscard]] bool IsValidDirectoryName(std::string_view name) const noexcept;
		[[nodiscard]] const std::expected<std::filesystem::path, std::string> GetExecutableDirectory() const noexcept;
		inline const std::filesystem::path GetUserDataDirectory() const noexcept { return m_UserDataDirectory; };

	private:
		std::string m_AppName;
		std::shared_ptr<zzz::engine::NativeAppData> m_NativeData;
		std::filesystem::path m_UserDataDirectory;

		[[nodiscard]] std::expected<std::filesystem::path, std::string> ResolveUserDataDirectory(std::string_view appName);

#if Z_MACOS || Z_IOS
		[[nodiscard]] std::expected<std::filesystem::path, std::string> GetAppleUserDataDirectory();
#endif
	};
}
