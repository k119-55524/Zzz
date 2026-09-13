#pragma once

#include <string>
#include <string_view>
#include "core/Serialize/Serializer.h"
#include "core/Enums/platforms/eAndroidEnums.h"

namespace zzz::core
{

	class ViewDataAndroid final : public ISerializable
	{
	public:
		ViewDataAndroid() : isPrimary(false) {}
		ViewDataAndroid(eAndroidScreenOrientation orientation, zU32 targetFPS = 60, eAndroidCutoutMode cutoutMode = eAndroidCutoutMode::ShortEdges, bool keepScreenOn = true)
			: orientation(orientation)
			, targetFPS(targetFPS)
			, cutoutMode(cutoutMode)
			, keepScreenOn(keepScreenOn)
			, isPrimary(false)
		{}

		[[nodiscard]] eAndroidScreenOrientation GetOrientation() const noexcept { return orientation; }
		[[nodiscard]] zU32 GetTargetFPS() const noexcept { return targetFPS; }
		[[nodiscard]] eAndroidCutoutMode GetCutoutMode() const noexcept { return cutoutMode; }
		[[nodiscard]] bool IsKeepScreenOn() const noexcept { return keepScreenOn; }
		[[nodiscard]] bool IsPrimary() const noexcept { return isPrimary; }
		void SetPrimary(bool primary) noexcept { isPrimary = primary; }

		[[nodiscard]] bool operator==(const ViewDataAndroid& other) const noexcept
		{
			return orientation == other.orientation &&
				targetFPS == other.targetFPS &&
				cutoutMode == other.cutoutMode &&
				keepScreenOn == other.keepScreenOn;
		}

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			DOut(Assets, "{}[ViewDataAndroid]", indentation);
			DOut(Assets, "{}  orientation: {}", indentation, ToString(orientation));
			DOut(Assets, "{}  targetFPS: {}", indentation, targetFPS);
			DOut(Assets, "{}  cutoutMode: {}", indentation, ToString(cutoutMode));
			DOut(Assets, "{}  keepScreenOn: {}", indentation, keepScreenOn);
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, orientation)
				.and_then([&]() { return s.Serialize(buffer, targetFPS); })
				.and_then([&]() { return s.Serialize(buffer, cutoutMode); })
				.and_then([&]() { return s.Serialize(buffer, keepScreenOn); });
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, orientation)
				.and_then([&]() { return s.Deserialize(buffer, offset, targetFPS); })
				.and_then([&]() { return s.Deserialize(buffer, offset, cutoutMode); })
				.and_then([&]() { return s.Deserialize(buffer, offset, keepScreenOn); });
		}

		eAndroidScreenOrientation orientation = eAndroidScreenOrientation::LandscapeLeft;
		zU32 targetFPS = 60;
		eAndroidCutoutMode cutoutMode = eAndroidCutoutMode::ShortEdges;
		bool keepScreenOn = true;

		// Не сериализуется - транзиентная метка "кто создаёт это окно" (Primary или нет).
		// Проставляется заново каждый раз в UserSettingsManager::GetOrCreateXxxViewPlatformData()
		// при чтении/создании и не должна переживать сериализацию в user.dat.
		bool isPrimary;
	};
}
