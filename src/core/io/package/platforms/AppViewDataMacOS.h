#pragma once

#include <common/Templates/Size2D.h>
#include <core/Serialize/Serializer.h>
#include <core/Enums/platforms/eMacOSEnums.h>

namespace zzz::core
{
	using namespace zzz::common;
	using namespace zzz::io;

	class AppViewDataMacOS final : public ISerializable
	{
	public:
		AppViewDataMacOS() = default;
		AppViewDataMacOS(Size2D<zU32> defaultSize, eMacOSWindowMode windowMode = eMacOSWindowMode::Windowed, bool resizable = true)
			: defaultSize(defaultSize)
			, windowMode(windowMode)
			, resizable(resizable)
		{}

		[[nodiscard]] const Size2D<zU32>& GetDefaultSize() const noexcept { return defaultSize; }
		[[nodiscard]] eMacOSWindowMode GetWindowMode() const noexcept { return windowMode; }
		[[nodiscard]] bool IsResizable() const noexcept { return resizable; }

		inline void LogFileBlock() const
		{
			DOut("           [AppViewDataMacOS] defaultSize: {}x{}", defaultSize.width, defaultSize.height);
			DOut("           [AppViewDataMacOS] windowMode: {}", EnumToString::ToString(windowMode));
			DOut("           [AppViewDataMacOS] resizable: {}", resizable);
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
		eMacOSWindowMode windowMode = eMacOSWindowMode::Windowed;
		bool resizable = true;
	};
}
