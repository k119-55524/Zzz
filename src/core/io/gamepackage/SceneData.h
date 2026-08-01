#pragma once

#include <string>
#include <vector>
#include <core/Guid.h>
#include <core/serialize/Serializer.h>

using namespace zzz::io;
using namespace zzz::common;

namespace zzz::core
{
	class SceneData final : public ISerializable
	{
	public:
		SceneData() = default;
		SceneData(std::string name, std::vector<Guid> sceneScriptGuids)
			: name(std::move(name))
			, sceneScriptGuids(std::move(sceneScriptGuids))
		{}

		std::string name;
		std::vector<Guid> sceneScriptGuids;

		void LogFileBlock() const
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			DOut("           [SceneData] Сцена: '{}'", name);
			DOut("           [SceneData] Скрипты({})", sceneScriptGuids.size());
			for (zU32 i = 0; i < sceneScriptGuids.size(); ++i)
			{
				DOut("             Script #{}: {}", i, sceneScriptGuids[i].ToString());
			}
#endif
		}

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			const zU32 scriptsCount = static_cast<zU32>(sceneScriptGuids.size());

			return serializer.Serialize(buffer, name)
				.and_then([&]() { return serializer.Serialize(buffer, scriptsCount); })
				.and_then([&]() -> std::expected<void, std::string> {
					for (const auto& scriptGuid : sceneScriptGuids)
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
				.and_then([&]() { return serializer.Deserialize(buffer, offset, scriptsCount); })
				.and_then([&]() -> std::expected<void, std::string> {
					sceneScriptGuids.clear();
					sceneScriptGuids.reserve(scriptsCount);
					for (zU32 i = 0; i < scriptsCount; ++i)
					{
						Guid scriptGuid{};
						auto res = serializer.Deserialize(buffer, offset, scriptGuid);
						if (!res) return res;
						sceneScriptGuids.push_back(scriptGuid);
					}
					return {};
				});
		}
	};
}
