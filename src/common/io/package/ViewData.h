#pragma once

#include <string>
#include <vector>
#include <common/guid.h>
#include <common/templates/Size2D.h>
#include <common/serialize/Serializer.h>

using namespace zzz::io;
using namespace zzz::common;

namespace zzz::core
{
	class ViewData final : public ISerializable
	{
	public:
		ViewData() = default;
		ViewData(std::string name, Size2D<zU32> size, Guid sceneGuid, std::vector<Guid> uiScriptGuids)
			: name(std::move(name))
			, size(size)
			, sceneGuid(sceneGuid)
			, uiScriptGuids(std::move(uiScriptGuids))
		{}

		std::string name;
		Size2D<zU32> size;
		Guid sceneGuid;
		std::vector<Guid> uiScriptGuids;

		void LogFileBlock() const
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			DOut("           [ViewData] Имя вида: '{}'", name);
			DOut("           [ViewData] Размер: {}x{}", size.width, size.height);
			DOut("           [ViewData] Привязанная сцена(GUID): {}", sceneGuid.ToString());
			DOut("           [ViewData] Скрипты({})", uiScriptGuids.size());
			for (zU32 i = 0; i < uiScriptGuids.size(); ++i)
			{
				DOut("             Script #{}: {}", i, uiScriptGuids[i].ToString());
			}
#endif
		}

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			return serializer.Serialize(buffer, name)
				.and_then([&]() { return serializer.Serialize(buffer, size); })
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

			return serializer.Deserialize(buffer, offset, name)
				.and_then([&]() { return serializer.Deserialize(buffer, offset, size); })
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
