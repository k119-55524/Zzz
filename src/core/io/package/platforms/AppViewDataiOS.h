#pragma once

#include <string_view>
#include <core/Serialize/Serializer.h>
#include <core/Enums/eEnumToString.h>
#include <core/Enums/platforms/eiOSEnums.h>

namespace zzz::core
{
	using namespace zzz::core;
	using namespace zzz::core;

	class AppViewDataiOS final : public ISerializable
	{
	public:
		AppViewDataiOS() = default;
		AppViewDataiOS(eiOSScreenOrientation orientation, eiOSSafeAreaMode safeAreaMode = eiOSSafeAreaMode::ExtendIntoSafeArea, eiOSHomeIndicatorMode homeIndicatorMode = eiOSHomeIndicatorMode::AutoHidden)
			: orientation(orientation)
			, safeAreaMode(safeAreaMode)
			, homeIndicatorMode(homeIndicatorMode)
		{}

		[[nodiscard]] eiOSScreenOrientation GetOrientation() const noexcept { return orientation; }
		[[nodiscard]] eiOSSafeAreaMode GetSafeAreaMode() const noexcept { return safeAreaMode; }
		[[nodiscard]] eiOSHomeIndicatorMode GetHomeIndicatorMode() const noexcept { return homeIndicatorMode; }

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			DOut("{}[AppViewDataiOS] orientation: {}", indentation, EnumToString::ToString(orientation));
			DOut("{}[AppViewDataiOS] safeAreaMode: {}", indentation, EnumToString::ToString(safeAreaMode));
			DOut("{}[AppViewDataiOS] homeIndicatorMode: {}", indentation, EnumToString::ToString(homeIndicatorMode));
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
