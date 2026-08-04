#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <core/Guid.h>
#include <common/Templates/Size2D.h>
#include <core/Serialize/Serializer.h>

using namespace zzz::io;
using namespace zzz::common;

namespace zzz::engine
{
	class PackageManager;
}

namespace zzz::core
{
	class ViewData final : public ISerializable
	{
	public:
		ViewData() = default;
		ViewData(Size2D<zU32> size, Guid sceneGuid, std::vector<Guid> uiScriptGuids)
			: size(size)
			, sceneGuid(sceneGuid)
			, uiScriptGuids(std::move(uiScriptGuids))
		{}

		[[nodiscard]] const std::string& GetName() const noexcept { return name; }
		[[nodiscard]] const Size2D<zU32>& GetSize() const noexcept { return size; }
		[[nodiscard]] const Guid& GetSceneGuid() const noexcept { return sceneGuid; }
		[[nodiscard]] const std::vector<Guid>& GetUiScriptGuids() const noexcept { return uiScriptGuids; }

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut("{}[ViewData] name: {}", indentation, name);
			DOut("{}[ViewData] size: {}x{}", indentation, size.width, size.height);
			DOut("{}[ViewData] sceneGuid: {}", indentation, sceneGuid.ToString());
			DOut("{}[ViewData] uiScriptGuids({})", indentation, uiScriptGuids.size());
			for (zU32 i = 0; i < uiScriptGuids.size(); ++i)
			{
				DOut("{}uiScriptGuid #{}: {}", nestedIndentation, i, uiScriptGuids[i].ToString());
			}
		}

	private:
		std::string name;
		Size2D<zU32> size;
		Guid sceneGuid;
		std::vector<Guid> uiScriptGuids;

		void SetName(std::string_view viewName) { name = viewName; }
		friend class zzz::engine::PackageManager;

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			return serializer.Serialize(buffer, size)
				.and_then([&]() { return serializer.Serialize(buffer, sceneGuid); })
				.and_then([&]() {
					const zU32 scriptsCount = static_cast<zU32>(uiScriptGuids.size());
					return serializer.Serialize(buffer, scriptsCount);
				})
				.and_then([&]() -> std::expected<void, std::string> {
					for (const auto& scriptGuid : uiScriptGuids)
					{
						auto res = serializer.Serialize(buffer, scriptGuid);
						if (!res) return res;
					}
					return {};
				});
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			zU32 scriptsCount = 0;

			return serializer.Deserialize(buffer, offset, size)
				.and_then([&]() { return serializer.Deserialize(buffer, offset, sceneGuid); })
				.and_then([&]() {
					return serializer.Deserialize(buffer, offset, scriptsCount);
				})
				.and_then([&]() -> std::expected<void, std::string> {
					uiScriptGuids.clear();
					uiScriptGuids.reserve(scriptsCount);
					for (zU32 i = 0; i < scriptsCount; ++i)
					{
						Guid scriptGuid{};
						auto res = serializer.Deserialize(buffer, offset, scriptGuid);
						if (!res) return res;
						uiScriptGuids.push_back(scriptGuid);
					}
					return {};
				});
		}
	};
}

