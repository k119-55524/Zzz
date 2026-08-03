#pragma once

#include <core/Serialize/Serializer.h>
#include <core/Enums/platforms/eAndroidEnums.h>

namespace zzz::core
{
	using namespace zzz::common;
	using namespace zzz::io;

	class AppViewDataAndroid final : public ISerializable
	{
	public:
		AppViewDataAndroid() = default;
		AppViewDataAndroid(eAndroidScreenOrientation orientation, zU32 targetFPS = 60, eAndroidCutoutMode cutoutMode = eAndroidCutoutMode::ShortEdges, bool keepScreenOn = true)
			: orientation(orientation)
			, targetFPS(targetFPS)
			, cutoutMode(cutoutMode)
			, keepScreenOn(keepScreenOn)
		{}

		[[nodiscard]] eAndroidScreenOrientation GetOrientation() const noexcept { return orientation; }
		[[nodiscard]] zU32 GetTargetFPS() const noexcept { return targetFPS; }
		[[nodiscard]] eAndroidCutoutMode GetCutoutMode() const noexcept { return cutoutMode; }
		[[nodiscard]] bool KeepScreenOn() const noexcept { return keepScreenOn; }

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
	};
}
