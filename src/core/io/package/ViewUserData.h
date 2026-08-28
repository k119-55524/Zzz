#pragma once

#include "core/utils/Guid.h"
#include "core/hardware/MonitorInfo.h"
#include "core/Serialize/Serializer.h"
#include "core/enums/eEnumToString.h"
#include "core/io/package/platforms/start_view/ViewPlatformConfig.h"
#include "engine/view/ViewWindowState.h"

namespace zzz::core
{

	/**
	 * @brief Сохраненное пользовательское состояние конкретного окна (View) в user.dat.
	 */
	class ViewUserData final : public ISerializable
	{
	public:
		ViewUserData() = default;
		explicit ViewUserData(zzz::engine::ViewWindowState state)
			: m_State(std::move(state))
		{
			SyncPlatformDataFromState();
		}

		[[nodiscard]] const Guid& GetViewGuid() const noexcept { return m_State.GetViewGuid(); }
		[[nodiscard]] const zzz::engine::ViewWindowState& GetWindowState() const noexcept { return m_State; }
		[[nodiscard]] zzz::engine::ViewWindowState& GetWindowState() noexcept { return m_State; }

		[[nodiscard]] ViewPlatformData& GetPlatformDataRef() noexcept
		{
			return m_PlatformData;
		}

		[[nodiscard]] inline ViewPlatformData GetPlatformData() const noexcept
		{
			return m_PlatformData;
		}

		void SyncStateFromPlatformData()
		{
			auto& navState = m_State.GetNativeState();
			navState.SetWindowRect(m_PlatformData.GetWindowRect());
			navState.SetState(m_PlatformData.GetWindowState());
			navState.SetMonitorId(m_PlatformData.GetMonitorId());
		}

		void SyncPlatformDataFromState()
		{
			const auto& navState = m_State.GetNativeState();
			m_PlatformData.SetWindowRect(navState.GetWindowRect());
			m_PlatformData.SetWindowState(navState.GetState());
			m_PlatformData.SetMonitorId(navState.GetMonitorId());
		}

		[[nodiscard]] bool operator==(const ViewUserData& other) const noexcept
		{
			return m_State == other.m_State;
		}

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut("{}[ViewUserData]", indentation);
			DOut("{}viewGuid: {}", nestedIndentation, m_State.GetViewGuid().ToString());
			DOut("{}windowState: {}", nestedIndentation, EnumToString::ToString(m_PlatformData.GetWindowState()));
			DOut("{}platformMonitorId: {}", nestedIndentation, m_PlatformData.GetMonitorId());
			DOut("{}windowRect: {}", nestedIndentation, m_PlatformData.GetWindowRect().ToString());
#endif
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			const_cast<ViewUserData*>(this)->SyncStateFromPlatformData();
			return s.Serialize(buffer, m_State);
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			auto res = s.Deserialize(buffer, offset, m_State);
			if (res)
				SyncPlatformDataFromState();
			return res;
		}

		zzz::engine::ViewWindowState m_State;
		ViewPlatformData m_PlatformData;
	};
}
