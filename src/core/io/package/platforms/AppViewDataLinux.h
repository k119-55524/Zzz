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
		AppViewDataLinux(Size2D<zU32> defaultSize, eLinuxWindowMode windowMode = eLinuxWindowMode::Windowed, bool resizable = true, eLinuxDisplayServer displayServer = eLinuxDisplayServer::Auto)
			: defaultSize(defaultSize)
			, windowMode(windowMode)
			, resizable(resizable)
			, displayServer(displayServer)
		{}

		[[nodiscard]] const Size2D<zU32>& GetDefaultSize() const noexcept { return defaultSize; }
		[[nodiscard]] eLinuxWindowMode GetWindowMode() const noexcept { return windowMode; }
		[[nodiscard]] bool IsResizable() const noexcept { return resizable; }
		[[nodiscard]] eLinuxDisplayServer GetDisplayServer() const noexcept { return displayServer; }

		inline void LogFileBlock() const
		{
			DOut("           [AppViewDataLinux] defaultSize: {}x{}", defaultSize.width, defaultSize.height);
			DOut("           [AppViewDataLinux] windowMode: {}", EnumToString::ToString(windowMode));
			DOut("           [AppViewDataLinux] resizable: {}", resizable);
			DOut("           [AppViewDataLinux] displayServer: {}", EnumToString::ToString(displayServer));
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, defaultSize)
				.and_then([&]() { return s.Serialize(buffer, windowMode); })
				.and_then([&]() { return s.Serialize(buffer, resizable); })
				.and_then([&]() { return s.Serialize(buffer, displayServer); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, defaultSize)
				.and_then([&]() { return s.Deserialize(buffer, offset, windowMode); })
				.and_then([&]() { return s.Deserialize(buffer, offset, resizable); })
				.and_then([&]() { return s.Deserialize(buffer, offset, displayServer); });
		}

		Size2D<zU32> defaultSize{ 1280, 720 };
		eLinuxWindowMode windowMode = eLinuxWindowMode::Windowed;
		bool resizable = true;
		eLinuxDisplayServer displayServer = eLinuxDisplayServer::Auto;
	};
}
