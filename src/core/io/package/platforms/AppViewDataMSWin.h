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
		AppViewDataMSWin(Size2D<zU32> defaultSize, eMSWinWindowMode windowMode = eMSWinWindowMode::Windowed, bool resizable = true)
			: defaultSize(defaultSize)
			, windowMode(windowMode)
			, resizable(resizable)
		{}

		[[nodiscard]] const Size2D<zU32>& GetDefaultSize() const noexcept { return defaultSize; }
		[[nodiscard]] eMSWinWindowMode GetWindowMode() const noexcept { return windowMode; }
		[[nodiscard]] bool IsResizable() const noexcept { return resizable; }

		inline void LogFileBlock() const
		{
			DOut("           [AppViewDataMSWin] defaultSize: {}x{}", defaultSize.width, defaultSize.height);
			DOut("           [AppViewDataMSWin] windowMode: {}", EnumToString::ToString(windowMode));
			DOut("           [AppViewDataMSWin] resizable: {}", resizable);
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, defaultSize)
				.and_then([&]() { return s.Serialize(buffer, windowMode); })
				.and_then([&]() { return s.Serialize(buffer, resizable); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, defaultSize)
				.and_then([&]() { return s.Deserialize(buffer, offset, windowMode); })
				.and_then([&]() { return s.Deserialize(buffer, offset, resizable); });
		}

		Size2D<zU32> defaultSize{ 1280, 720 };
		eMSWinWindowMode windowMode = eMSWinWindowMode::Windowed;
		bool resizable = true;
	};
}
