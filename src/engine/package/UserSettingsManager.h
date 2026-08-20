#pragma once

#include "engine/EngineIncludes.h"

using namespace zzz::core;

namespace zzz::engine
{
	class UserSettingsManager final : public ISerializable
	{
	public:
		UserSettingsManager() = delete;
		UserSettingsManager(const Path& path, const StartViewData& defaultStartViewData);

		[[nodiscard]] inline const StartViewUserData& GetStartViewUserData() const noexcept { return m_StartViewUserData; }
		[[nodiscard]] inline const HardwareState& GetHardwareState() const noexcept { return m_HardwareState; }

		void SetSelectedGpuId(std::string gpuId);
		void SetSelectedMonitorId(std::string monitorId);
		void UpdateStartViewData(zU32 monitorIndex, const std::vector<MonitorInfo>& availableMonitors);
		void UpdateStartWindowRect(const Rect2D<zI32>& rect);

		[[nodiscard]] std::expected<void, std::string> SaveConfig();

	private:
		void Initialize(const StartViewData& defaultStartViewData);
		void LogUserData() const;
		void SetDefaultUserSettings(const StartViewData& defaultStartViewData);
		std::expected<std::filesystem::path, std::string> GetSettingsDirectory();
		std::expected<void, std::string> LoadConfig(std::filesystem::path path);

		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;

		Path m_Path;
		std::filesystem::path m_ConfigPath;

		Version m_Version;
		StartViewUserData m_StartViewUserData;
		HardwareState m_HardwareState;

		bool m_IsDirty;
	};
}
