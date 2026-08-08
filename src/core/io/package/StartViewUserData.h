#pragma once

#include <string>
#include <string_view>
#include "core/Serialize/Serializer.h"
#include "core/IO/package/StartViewData.h"

namespace zzz::core
{
	class StartViewUserData final : public ISerializable
	{
	public:
		StartViewUserData() = default;
		explicit StartViewUserData(const StartViewData& startViewData)
			: m_PlatformData(startViewData.GetPlatformData())
		{}

		[[nodiscard]] const StartViewPlatformData& GetPlatformData() const noexcept { return m_PlatformData; }

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			DOut("{}[StartViewUserData]", indentation);
			m_PlatformData.LogFileBlock(std::string(indentation) + "  ");
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, m_PlatformData);
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, m_PlatformData);
		}

		StartViewPlatformData m_PlatformData;
	};
}
