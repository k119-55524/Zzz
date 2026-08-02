#pragma once

#include <core/Version.h>
#include <logger/logger.h>
#include "PlatformConfig.h"
#include <core/Serialize/Serializer.h>
#include <core/IO/package/AppViewUserData.h>

namespace zzz::core
{
	class AppViewData;
}

namespace zzz::engine
{
	using namespace zzz::common;
	class UserSettings final : public ISerializable
	{
	public:
		explicit UserSettings();
		explicit UserSettings(const zzz::core::AppViewData& appViewData);
		~UserSettings() = default;

		inline const AppViewUserData& GetAppViewUserData() const noexcept { return m_AppViewUserData; }
		inline const PlatformConfig& GetPlatformConfig() const noexcept { return m_PlatformConfig; }

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;

		Version m_Version;
		AppViewUserData m_AppViewUserData;
		PlatformConfig m_PlatformConfig;
	};
}
