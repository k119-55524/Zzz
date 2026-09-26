#pragma once

#include <memory>
#include <string>
#include <expected>
#include <filesystem>
#include <string_view>

#include "core/utils/NativeAppData.h"
#include "core/enums/eFileLocation.h"
#include "core/enums/eDataDatType.h"

namespace zzz::core
{
	class Path final
	{
	public:
		Path() = delete;
		explicit Path(std::shared_ptr<NativeAppData> nativeData = nullptr);

		[[nodiscard]] std::expected<std::filesystem::path, std::string> GetDirectory(eFileLocation location) const noexcept;
		[[nodiscard]] static bool IsValidDirectoryName(std::string_view name) noexcept;
		[[nodiscard]] static std::filesystem::path ResolvePakPath(eDataDatType type) noexcept;
		[[nodiscard]] std::expected<void, std::string> InitializeUserData(std::string_view companyName, std::string_view appName);

	private:
		[[nodiscard]] static std::expected<std::filesystem::path, std::string> ResolveExecutableDirectory() noexcept;
		[[nodiscard]] std::expected<std::filesystem::path, std::string> ResolveUserDataDirectory(std::string_view companyName, std::string_view appName);

		std::shared_ptr<NativeAppData> m_NativeData;
		std::filesystem::path m_ExecutableDirectory;
		std::filesystem::path m_UserDataDirectory;

#if defined(Z_APPLE)
		[[nodiscard]] std::expected<std::filesystem::path, std::string> GetAppleUserDataDirectory();
#endif
	};
}
