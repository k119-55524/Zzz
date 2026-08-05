#pragma once

#include "engine/EngineIncludes.h"

namespace zzz::engine
{
	class PackageManager;

	class UserSettingsManager final : public ISerializable
	{
	public:
		UserSettingsManager() = delete;
		UserSettingsManager(const Path& path, const PackageManager& packageManager);

		inline const AppViewUserData& GetAppViewUserData() const noexcept { return m_AppViewUserData; }
		inline const PlatformConfig& GetPlatformConfig() const noexcept { return m_PlatformConfig; }

		[[nodiscard]] std::expected<void, std::string> SaveConfig();

	private:
		void Initialize(const PackageManager& packageManager);
		void LogUserData() const;
		void SetDefaultUserSettings(const PackageManager& packageManager);
		std::expected<std::filesystem::path, std::string> GetSettingsDirectory();
		std::expected<void, std::string> LoadConfig(std::filesystem::path path);

		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;

		Path m_Path;
		std::filesystem::path m_ConfigPath;

		Version m_Version;
		AppViewUserData m_AppViewUserData;
		PlatformConfig m_PlatformConfig;

		Serializer m_Serializer;
		bool m_IsDirty;
	};
}
