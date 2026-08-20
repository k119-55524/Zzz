#pragma once

#include <unordered_map>
#include "engine/EngineIncludes.h"
#include "core/io/package/ViewUserData.h"

using namespace zzz::core;

namespace zzz::engine
{
	class View;

	using ViewUserDataMap = std::unordered_map<Guid, ViewUserData>;

	class UserSettingsManager final : public ISerializable
	{
	public:
		UserSettingsManager() = delete;
		UserSettingsManager(const Path& path, const PrimaryViewData& defaultPrimaryViewData);

		[[nodiscard]] inline const PrimaryViewUserData& GetPrimaryViewUserData() const noexcept { return m_PrimaryViewUserData; }
		[[nodiscard]] inline const HardwareState& GetHardwareState() const noexcept { return m_HardwareState; }
		[[nodiscard]] inline const ViewUserDataMap& GetChildViewsUserData() const noexcept { return m_ChildViewsUserData; }
		[[nodiscard]] inline const ViewUserDataMap& GetIndependentViewsUserData() const noexcept { return m_IndependentViewsUserData; }

		void SetSelectedGpuId(std::string gpuId);
		void SetSelectedMonitorId(std::string monitorId);
		void StoreViewState(const View& view);

		[[nodiscard]] std::expected<void, std::string> SaveConfig();

	private:
		void Initialize(const PrimaryViewData& defaultPrimaryViewData);
		void LogUserData() const;
		void SetDefaultUserSettings(const PrimaryViewData& defaultPrimaryViewData);
		std::expected<std::filesystem::path, std::string> GetSettingsDirectory();
		std::expected<void, std::string> LoadConfig(std::filesystem::path path);

		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;

		Path m_Path;
		std::filesystem::path m_ConfigPath;

		Version m_Version;
		PrimaryViewUserData m_PrimaryViewUserData;
		ViewUserDataMap m_ChildViewsUserData;
		ViewUserDataMap m_IndependentViewsUserData;
		HardwareState m_HardwareState;

		bool m_IsDirty;
	};
}
