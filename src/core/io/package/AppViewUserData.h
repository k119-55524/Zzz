#pragma once

#include <core/Serialize/Serializer.h>
#include <core/IO/package/AppViewData.h>

namespace zzz::core
{
	class AppViewUserData final : public ISerializable
	{
	public:
		AppViewUserData() = default;
		explicit AppViewUserData(const AppViewData& appViewData)
			: m_PlatformData(appViewData.GetPlatformData())
		{}

		[[nodiscard]] const AppViewPlatformData& GetPlatformData() const noexcept { return m_PlatformData; }

		inline void LogFileBlock() const
		{
			DOut("           [AppViewUserData]");
			m_PlatformData.LogFileBlock();
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

		AppViewPlatformData m_PlatformData;
	};
}
