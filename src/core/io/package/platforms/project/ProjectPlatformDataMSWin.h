#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <logger/logger.h>
#include "core/Serialize/Serializer.h"

namespace zzz::core
{
	class ProjectPlatformDataMSWin final : public ISerializable
	{
	public:
		ProjectPlatformDataMSWin() = default;
		explicit ProjectPlatformDataMSWin(std::string windowClassName)
			: windowClassName(std::move(windowClassName))
		{}

		[[nodiscard]] const std::string& GetWindowClassName() const noexcept { return windowClassName; }

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut(::zzz::core::Assets, "{}[ProjectPlatformDataMSWin]", indentation);
			DOut(::zzz::core::Assets, "{}windowClassName: {}", nestedIndentation, windowClassName);
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, windowClassName);
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, windowClassName);
		}

		std::string windowClassName{ "ZzzEngineWindowClass" };
	};
}
