#pragma once

#include <string>
#include <string_view>
#include "core/Serialize/Serializer.h"
#include "core/Enums/eEnumToString.h"
#include "core/Enums/platforms/eiOSEnums.h"

namespace zzz::core
{

	class ViewDataiOS final : public ISerializable
	{
	public:
		ViewDataiOS() : isPrimary(false) {}
		ViewDataiOS(eiOSScreenOrientation orientation, eiOSSafeAreaMode safeAreaMode = eiOSSafeAreaMode::ExtendIntoSafeArea, eiOSHomeIndicatorMode homeIndicatorMode = eiOSHomeIndicatorMode::AutoHidden)
			: orientation(orientation)
			, safeAreaMode(safeAreaMode)
			, homeIndicatorMode(homeIndicatorMode)
			, isPrimary(false)
		{}

		[[nodiscard]] eiOSScreenOrientation GetOrientation() const noexcept { return orientation; }
		[[nodiscard]] eiOSSafeAreaMode GetSafeAreaMode() const noexcept { return safeAreaMode; }
		[[nodiscard]] eiOSHomeIndicatorMode GetHomeIndicatorMode() const noexcept { return homeIndicatorMode; }
		[[nodiscard]] bool IsPrimary() const noexcept { return isPrimary; }
		void SetPrimary(bool primary) noexcept { isPrimary = primary; }

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut(::zzz::core::Assets, "{}[StartViewDataiOS]", indentation);
			DOut(::zzz::core::Assets, "{}orientation: {}", nestedIndentation, EnumToString::ToString(orientation));
			DOut(::zzz::core::Assets, "{}safeAreaMode: {}", nestedIndentation, EnumToString::ToString(safeAreaMode));
			DOut(::zzz::core::Assets, "{}homeIndicatorMode: {}", nestedIndentation, EnumToString::ToString(homeIndicatorMode));
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

		// Не сериализуется - транзиентная метка "кто создаёт это окно" (Primary или нет).
		// Проставляется заново каждый раз в UserSettingsManager::GetOrCreateXxxViewPlatformData()
		// при чтении/создании и не должна переживать сериализацию в user.dat.
		bool isPrimary;
	};
}
