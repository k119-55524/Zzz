#pragma once

#include <string>
#include <string_view>
#include "core/Serialize/Serializer.h"
#include "core/IO/package/PrimaryViewData.h"

namespace zzz::core
{
	class PrimaryViewUserData final : public ISerializable
	{
	public:
		PrimaryViewUserData() = default;
		PrimaryViewUserData(Guid viewGuid, ViewPlatformData platformData)
			: m_ViewGuid(std::move(viewGuid))
			, m_PlatformData(std::move(platformData))
		{}
		explicit PrimaryViewUserData(const PrimaryViewData& primaryViewData)
			: m_ViewGuid(primaryViewData.GetViewGuid())
			, m_PlatformData(primaryViewData.GetPlatformData())
		{}

		[[nodiscard]] const Guid& GetViewGuid() const noexcept { return m_ViewGuid; }
		[[nodiscard]] const ViewPlatformData& GetPlatformData() const noexcept { return m_PlatformData; }
		[[nodiscard]] ViewPlatformData& GetPlatformData() noexcept { return m_PlatformData; }

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			DOut(::zzz::core::Assets, "{}[PrimaryViewUserData]", indentation);
			DOut(::zzz::core::Assets, "{}viewGuid: {}", std::string(indentation) + "  ", m_ViewGuid.ToString());
			m_PlatformData.LogFileBlock(std::string(indentation) + "  ");
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, m_ViewGuid)
				.and_then([&]() { return s.Serialize(buffer, m_PlatformData); });
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, m_ViewGuid)
				.and_then([&]() { return s.Deserialize(buffer, offset, m_PlatformData); });
		}

		Guid m_ViewGuid;
		ViewPlatformData m_PlatformData;
	};
}
