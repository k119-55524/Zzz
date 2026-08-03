#pragma once

#include <common/Templates/Size2D.h>
#include <core/Serialize/Serializer.h>
#include <core/Enums/platforms/eMSWinEnums.h>

namespace zzz::core
{
	using namespace zzz::common;
	using namespace zzz::io;

	class AppViewDataMSWin final : public ISerializable
	{
	public:
		AppViewDataMSWin() = default;
		AppViewDataMSWin(Size2D<zU32> defaultSize, bool isFullscreenByDefault = false, bool resizable = true, eMSWinWindowMode windowMode = eMSWinWindowMode::Windowed)
			: defaultSize(defaultSize)
			, isFullscreenByDefault(isFullscreenByDefault)
			, resizable(resizable)
			, windowMode(windowMode)
		{}

		[[nodiscard]] const Size2D<zU32>& GetDefaultSize() const noexcept { return defaultSize; }
		[[nodiscard]] bool IsFullscreenByDefault() const noexcept { return isFullscreenByDefault; }
		[[nodiscard]] bool IsResizable() const noexcept { return resizable; }
		[[nodiscard]] eMSWinWindowMode GetWindowMode() const noexcept { return windowMode; }

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, defaultSize)
				.and_then([&]() { return s.Serialize(buffer, isFullscreenByDefault); })
				.and_then([&]() { return s.Serialize(buffer, resizable); })
				.and_then([&]() { return s.Serialize(buffer, windowMode); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, defaultSize)
				.and_then([&]() { return s.Deserialize(buffer, offset, isFullscreenByDefault); })
				.and_then([&]() { return s.Deserialize(buffer, offset, resizable); })
				.and_then([&]() { return s.Deserialize(buffer, offset, windowMode); });
		}

		Size2D<zU32> defaultSize{ 1280, 720 };
		bool isFullscreenByDefault = false;
		bool resizable = true;
		eMSWinWindowMode windowMode = eMSWinWindowMode::Windowed;
	};
}
