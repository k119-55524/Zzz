#pragma once

#include <span>
#include <vector>
#include "core/utils/Guid.h"
#include "core/Serialize/Serializer.h"
#include "engine/platforms/window/NativeWindowState.h"

namespace zzz::engine
{
	class ViewWindowState final : public zzz::core::ISerializable
	{
	public:
		ViewWindowState() = default;
		ViewWindowState(zzz::core::Guid viewGuid, NativeWindowState nativeState)
			: m_ViewGuid(std::move(viewGuid))
			, m_NativeState(std::move(nativeState))
		{}

		[[nodiscard]] const zzz::core::Guid& GetViewGuid() const noexcept { return m_ViewGuid; }
		[[nodiscard]] const NativeWindowState& GetNativeState() const noexcept { return m_NativeState; }
		[[nodiscard]] NativeWindowState& GetNativeState() noexcept { return m_NativeState; }

		[[nodiscard]] bool operator==(const ViewWindowState& other) const noexcept
		{
			return m_ViewGuid == other.m_ViewGuid &&
				m_NativeState == other.m_NativeState;
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const zzz::core::Serializer& s) const override
		{
			return s.Serialize(buffer, m_ViewGuid)
				.and_then([&]() { return s.Serialize(buffer, m_NativeState); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const zzz::core::Serializer& s) override
		{
			return s.Deserialize(buffer, offset, m_ViewGuid)
				.and_then([&]() { return s.Deserialize(buffer, offset, m_NativeState); });
		}

		zzz::core::Guid m_ViewGuid;
		NativeWindowState m_NativeState;
	};
}
