#pragma once

#include "core/utils/Guid.h"
#include "core/hardware/MonitorInfo.h"
#include "core/Serialize/Serializer.h"
#include "core/enums/eEnumToString.h"
#include "engine/view/ViewWindowState.h"

namespace zzz::core
{
	using namespace zzz::engine;

	/**
	 * @brief Сохраненное пользовательское состояние конкретного окна (View) в user.dat.
	 */
	class ViewUserData final : public ISerializable
	{
	public:
		ViewUserData() = default;
		explicit ViewUserData(ViewWindowState state)
			: m_State(std::move(state))
		{}

		[[nodiscard]] const Guid& GetViewGuid() const noexcept { return m_State.GetViewGuid(); }
		[[nodiscard]] const ViewWindowState& GetWindowState() const noexcept { return m_State; }
		[[nodiscard]] ViewWindowState& GetWindowState() noexcept { return m_State; }

		[[nodiscard]] inline ViewPlatformData GetPlatformData() const noexcept
		{
			const auto& navState = m_State.GetNativeState();
			ViewPlatformData platformData;
			platformData.SetWindowRect(navState.GetWindowRect());
			platformData.SetWindowState(navState.GetState());
			platformData.SetMonitorId(navState.GetMonitorId());
			return platformData;
		}

		[[nodiscard]] bool operator==(const ViewUserData& other) const noexcept
		{
			return m_State == other.m_State;
		}

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			const std::string nestedIndentation = std::string(indentation) + "  ";
			const auto& navState = m_State.GetNativeState();
			DOut("{}[ViewUserData]", indentation);
			DOut("{}viewGuid: {}", nestedIndentation, m_State.GetViewGuid().ToString());
			DOut("{}windowState: {}", nestedIndentation, EnumToString::ToString(navState.GetState()));
			DOut("{}platformMonitorId: {}", nestedIndentation, navState.GetMonitorId());
			DOut("{}windowRect: {}", nestedIndentation, navState.GetWindowRect().ToString());
#endif
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, m_State);
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, m_State);
		}

		ViewWindowState m_State;
	};
}
