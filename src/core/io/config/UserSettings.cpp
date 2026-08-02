
#include "UserSettings.h"
#include <core/Constants.h>

using namespace zzz::io;
using namespace zzz::engine;

UserSettings::UserSettings() :
	m_Version(c_ConfigFileMajorVersion, c_ConfigFileMinorVersion, c_ConfigFilePatchVersion)
{
}

UserSettings::UserSettings(const zzz::core::AppViewData& appViewData) :
	m_Version(c_ConfigFileMajorVersion, c_ConfigFileMinorVersion, c_ConfigFilePatchVersion),
	m_AppViewUserData(appViewData)
{
}

[[nodiscard]] std::expected<void, std::string> UserSettings::Serialize(std::vector<std::byte>& buffer, const Serializer& s) const
{
	return s.Serialize(buffer, c_ConfigHeader)
		.and_then([&]() { return s.Serialize(buffer, m_Version); })
		.and_then([&]() { return s.Serialize(buffer, m_AppViewUserData); })
		.and_then([&]() { return s.Serialize(buffer, m_PlatformConfig); });
}

[[nodiscard]] std::expected<void, std::string> UserSettings::Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s)
{
	FileHeader<3> header{};

	return s.Deserialize(buffer, offset, header)
		.and_then([&]() -> std::expected<void, std::string>
			{
				if (header != c_ConfigHeader)
					return UNEXPECTED("Некорректный заголовок конфигурации.");

				return s.Deserialize(buffer, offset, m_Version);
			})
		.and_then([&]() { return s.Deserialize(buffer, offset, m_AppViewUserData); })
		.and_then([&]() { return s.Deserialize(buffer, offset, m_PlatformConfig); });
}
