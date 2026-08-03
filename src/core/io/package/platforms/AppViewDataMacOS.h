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
		AppViewDataMacOS(Size2D<zU32> defaultSize, bool isFullscreenByDefault = false, bool resizable = true)
			: defaultSize(defaultSize)
			, isFullscreenByDefault(isFullscreenByDefault)
			, resizable(resizable)
		{}

		[[nodiscard]] const Size2D<zU32>& GetDefaultSize() const noexcept { return defaultSize; }
		[[nodiscard]] bool IsFullscreenByDefault() const noexcept { return isFullscreenByDefault; }
		[[nodiscard]] bool IsResizable() const noexcept { return resizable; }

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, defaultSize)
				.and_then([&]() { return s.Serialize(buffer, isFullscreenByDefault); })
				.and_then([&]() { return s.Serialize(buffer, resizable); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, defaultSize)
				.and_then([&]() { return s.Deserialize(buffer, offset, isFullscreenByDefault); })
				.and_then([&]() { return s.Deserialize(buffer, offset, resizable); });
		}

		Size2D<zU32> defaultSize{ 1280, 720 };
		bool isFullscreenByDefault = false;
		bool resizable = true;
	};
}
