#pragma once

#include <string_view>
#include <core/templates/Size2D.h>
#include <core/Serialize/Serializer.h>
#include <core/Enums/eEnumToString.h>
#include <core/Enums/platforms/eLinuxEnums.h>

namespace zzz::core
{
	using namespace zzz::core;
	using namespace zzz::core;

	class AppViewDataLinux final : public ISerializable
	{
	public:
		AppViewDataLinux() = default;
		AppViewDataLinux(Size2D<zU32> size, eLinuxWindowMode windowMode = eLinuxWindowMode::Windowed, bool resizable = true, eLinuxDisplayServer displayServer = eLinuxDisplayServer::Auto)
			: size(size)
			, windowMode(windowMode)
			, resizable(resizable)
			, displayServer(displayServer)
		{}

		[[nodiscard]] const Size2D<zU32>& GetSize() const noexcept { return size; }
		[[nodiscard]] eLinuxWindowMode GetWindowMode() const noexcept { return windowMode; }
		[[nodiscard]] bool IsResizable() const noexcept { return resizable; }
		[[nodiscard]] eLinuxDisplayServer GetDisplayServer() const noexcept { return displayServer; }

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			DOut("{}[AppViewDataLinux] size: {}x{}", indentation, size.width, size.height);
			DOut("{}[AppViewDataLinux] windowMode: {}", indentation, EnumToString::ToString(windowMode));
			DOut("{}[AppViewDataLinux] resizable: {}", indentation, resizable);
			DOut("{}[AppViewDataLinux] displayServer: {}", indentation, EnumToString::ToString(displayServer));
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, size)
				.and_then([&]() { return s.Serialize(buffer, windowMode); })
				.and_then([&]() { return s.Serialize(buffer, resizable); })
				.and_then([&]() { return s.Serialize(buffer, displayServer); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, size)
				.and_then([&]() { return s.Deserialize(buffer, offset, windowMode); })
				.and_then([&]() { return s.Deserialize(buffer, offset, resizable); })
				.and_then([&]() { return s.Deserialize(buffer, offset, displayServer); });
		}

		Size2D<zU32> size{ 1280, 720 };
		eLinuxWindowMode windowMode = eLinuxWindowMode::Windowed;
		bool resizable = true;
		eLinuxDisplayServer displayServer = eLinuxDisplayServer::Auto;
	};
}
