#pragma once

#include <string>
#include <string_view>
#include "math/Size2D.h"
#include "core/Serialize/Serializer.h"
#include "core/Enums/eEnumToString.h"
#include "core/Enums/platforms/eMSWinEnums.h"

namespace zzz::core
{
	using namespace zzz::math;

	class StartViewDataMSWin final : public ISerializable
	{
	public:
		StartViewDataMSWin() = default;
		StartViewDataMSWin(std::string title, Size2D<zU32> size, eMSWinWindowMode windowMode = eMSWinWindowMode::Windowed, bool resizable = true)
			: title(std::move(title))
			, size(size)
			, windowMode(windowMode)
			, resizable(resizable)
		{}

		[[nodiscard]] const std::string& GetTitle() const noexcept { return title; }
		[[nodiscard]] const Size2D<zU32>& GetSize() const noexcept { return size; }
		[[nodiscard]] eMSWinWindowMode GetWindowMode() const noexcept { return windowMode; }
		[[nodiscard]] bool IsResizable() const noexcept { return resizable; }

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut("{}[StartViewDataMSWin]", indentation);
			DOut("{}title: {}", nestedIndentation, title);
			DOut("{}size: {}x{}", nestedIndentation, size.GetWidth(), size.GetHeight());
			DOut("{}windowMode: {}", nestedIndentation, EnumToString::ToString(windowMode));
			DOut("{}resizable: {}", nestedIndentation, resizable);
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, title)
				.and_then([&]() { return s.Serialize(buffer, size); })
				.and_then([&]() { return s.Serialize(buffer, windowMode); })
				.and_then([&]() { return s.Serialize(buffer, resizable); });
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, title)
				.and_then([&]() { return s.Deserialize(buffer, offset, size); })
				.and_then([&]() { return s.Deserialize(buffer, offset, windowMode); })
				.and_then([&]() { return s.Deserialize(buffer, offset, resizable); });
		}

		std::string title{ "Game Window" };
		Size2D<zU32> size{ 1280, 720 };
		eMSWinWindowMode windowMode = eMSWinWindowMode::Windowed;
		bool resizable = true;
	};
}
