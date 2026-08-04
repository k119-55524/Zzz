#pragma once

#include <string_view>
#include <core/templates/Size2D.h>
#include <core/Serialize/Serializer.h>
#include <core/Enums/eEnumToString.h>
#include <core/Enums/platforms/eMSWinEnums.h>

namespace zzz::core
{
	using namespace zzz::common;
	using namespace zzz::io;

	class AppViewDataMSWin final : public ISerializable
	{
	public:
		AppViewDataMSWin() = default;
		AppViewDataMSWin(Size2D<zU32> size, eMSWinWindowMode windowMode = eMSWinWindowMode::Windowed, bool resizable = true)
			: size(size)
			, windowMode(windowMode)
			, resizable(resizable)
		{}

		[[nodiscard]] const Size2D<zU32>& GetSize() const noexcept { return size; }
		[[nodiscard]] eMSWinWindowMode GetWindowMode() const noexcept { return windowMode; }
		[[nodiscard]] bool IsResizable() const noexcept { return resizable; }

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			DOut("{}[AppViewDataMSWin] size: {}x{}", indentation, size.width, size.height);
			DOut("{}[AppViewDataMSWin] windowMode: {}", indentation, EnumToString::ToString(windowMode));
			DOut("{}[AppViewDataMSWin] resizable: {}", indentation, resizable);
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, size)
				.and_then([&]() { return s.Serialize(buffer, windowMode); })
				.and_then([&]() { return s.Serialize(buffer, resizable); });
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, size)
				.and_then([&]() { return s.Deserialize(buffer, offset, windowMode); })
				.and_then([&]() { return s.Deserialize(buffer, offset, resizable); });
		}

		Size2D<zU32> size{ 1280, 720 };
		eMSWinWindowMode windowMode = eMSWinWindowMode::Windowed;
		bool resizable = true;
	};
}
