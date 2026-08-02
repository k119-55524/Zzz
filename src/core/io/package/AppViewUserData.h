#pragma once

#include <core/Serialize/Serializer.h>
#include <common/Templates/Size2D.h>
#include <core/IO/package/AppViewData.h>

namespace zzz::engine
{
	using namespace zzz::common;

	class AppViewUserData final : public ISerializable
	{
	public:
		AppViewUserData() = default;
		explicit AppViewUserData(const zzz::core::AppViewData& appViewData)
			: m_Size(appViewData.GetDefaultSize())
			, m_IsFullscreen(appViewData.IsFullscreenByDefault())
			, m_IsResizable(appViewData.IsResizable())
		{}

		[[nodiscard]] const Size2D<zU32>& GetSize() const noexcept { return m_Size; }
		[[nodiscard]] bool IsFullscreen() const noexcept { return m_IsFullscreen; }
		[[nodiscard]] bool IsResizable() const noexcept { return m_IsResizable; }

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, m_Size)
				.and_then([&]() { return s.Serialize(buffer, m_IsFullscreen); })
				.and_then([&]() { return s.Serialize(buffer, m_IsResizable); });
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, m_Size)
				.and_then([&]() { return s.Deserialize(buffer, offset, m_IsFullscreen); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_IsResizable); });
		}

		Size2D<zU32> m_Size;
		bool m_IsFullscreen = false;
		bool m_IsResizable = true;
	};
}
