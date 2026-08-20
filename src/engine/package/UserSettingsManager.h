#pragma once

#include "engine/EngineIncludes.h"
#include "core/io/package/ViewUserData.h"

using namespace zzz::core;

namespace zzz::engine
{
	class View;

	class UserSettingsManager final : public ISerializable
	{
	public:
		UserSettingsManager() = delete;
		UserSettingsManager(const Path& path, const StartViewData& defaultStartViewData);

		[[nodiscard]] inline const StartViewUserData& GetStartViewUserData() const noexcept { return m_StartViewUserData; }
		[[nodiscard]] inline const HardwareState& GetHardwareState() const noexcept { return m_HardwareState; }
		[[nodiscard]] const std::vector<ViewUserData>& GetViewsUserData() const noexcept { return m_ViewsUserData; }
		[[nodiscard]] const ViewUserData* FindViewUserData(const Guid& viewGuid) const noexcept;

		void SetSelectedGpuId(std::string gpuId);
		void SetSelectedMonitorId(std::string monitorId);
		void StoreViewState(const View& view);

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
		std::vector<ViewUserData> m_ViewsUserData;
		HardwareState m_HardwareState;

		bool m_IsDirty;
	};
}
