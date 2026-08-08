#pragma once

#include <string>
#include <string_view>
#include "core/Serialize/Serializer.h"
#include "core/Enums/eEnumToString.h"
#include "core/Enums/platforms/eiOSEnums.h"

namespace zzz::core
{

	class StartViewDataiOS final : public ISerializable
	{
	public:
		StartViewDataiOS() = default;
		StartViewDataiOS(eiOSScreenOrientation orientation, eiOSSafeAreaMode safeAreaMode = eiOSSafeAreaMode::ExtendIntoSafeArea, eiOSHomeIndicatorMode homeIndicatorMode = eiOSHomeIndicatorMode::AutoHidden)
			: orientation(orientation)
			, safeAreaMode(safeAreaMode)
			, homeIndicatorMode(homeIndicatorMode)
		{}

		[[nodiscard]] eiOSScreenOrientation GetOrientation() const noexcept { return orientation; }
		[[nodiscard]] eiOSSafeAreaMode GetSafeAreaMode() const noexcept { return safeAreaMode; }
		[[nodiscard]] eiOSHomeIndicatorMode GetHomeIndicatorMode() const noexcept { return homeIndicatorMode; }

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut("{}[StartViewDataiOS]", indentation);
			DOut("{}orientation: {}", nestedIndentation, EnumToString::ToString(orientation));
			DOut("{}safeAreaMode: {}", nestedIndentation, EnumToString::ToString(safeAreaMode));
			DOut("{}homeIndicatorMode: {}", nestedIndentation, EnumToString::ToString(homeIndicatorMode));
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, orientation)
				.and_then([&]() { return s.Serialize(buffer, safeAreaMode); })
				.and_then([&]() { return s.Serialize(buffer, homeIndicatorMode); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, orientation)
				.and_then([&]() { return s.Deserialize(buffer, offset, safeAreaMode); })
				.and_then([&]() { return s.Deserialize(buffer, offset, homeIndicatorMode); });
		}

		eiOSScreenOrientation orientation = eiOSScreenOrientation::LandscapeLeft;
		eiOSSafeAreaMode safeAreaMode = eiOSSafeAreaMode::ExtendIntoSafeArea;
		eiOSHomeIndicatorMode homeIndicatorMode = eiOSHomeIndicatorMode::AutoHidden;
	};
}
