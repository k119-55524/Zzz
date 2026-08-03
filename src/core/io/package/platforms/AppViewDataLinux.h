#pragma once

#include <common/Templates/Size2D.h>
#include <core/Serialize/Serializer.h>
#include <core/Enums/platforms/eLinuxEnums.h>

namespace zzz::core
{
	using namespace zzz::common;
	using namespace zzz::io;

	class AppViewDataLinux final : public ISerializable
	{
	public:
		AppViewDataLinux() = default;
		AppViewDataLinux(Size2D<zU32> defaultSize, bool isFullscreenByDefault = false, bool resizable = true, eLinuxDisplayServer displayServer = eLinuxDisplayServer::Auto)
			: defaultSize(defaultSize)
			, isFullscreenByDefault(isFullscreenByDefault)
			, resizable(resizable)
			, displayServer(displayServer)
		{}

		[[nodiscard]] const Size2D<zU32>& GetDefaultSize() const noexcept { return defaultSize; }
		[[nodiscard]] bool IsFullscreenByDefault() const noexcept { return isFullscreenByDefault; }
		[[nodiscard]] bool IsResizable() const noexcept { return resizable; }
		[[nodiscard]] eLinuxDisplayServer GetDisplayServer() const noexcept { return displayServer; }

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, defaultSize)
				.and_then([&]() { return s.Serialize(buffer, isFullscreenByDefault); })
				.and_then([&]() { return s.Serialize(buffer, resizable); })
				.and_then([&]() { return s.Serialize(buffer, displayServer); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, defaultSize)
				.and_then([&]() { return s.Deserialize(buffer, offset, isFullscreenByDefault); })
				.and_then([&]() { return s.Deserialize(buffer, offset, resizable); })
				.and_then([&]() { return s.Deserialize(buffer, offset, displayServer); });
		}

		Size2D<zU32> defaultSize{ 1280, 720 };
		bool isFullscreenByDefault = false;
		bool resizable = true;
		eLinuxDisplayServer displayServer = eLinuxDisplayServer::Auto;
	};
}
