#pragma once

#include <string>
#include <common/guid.h>
#include <common/io/IFileBlockLoggable.h>
#include <common/serialize/Serializer.h>

using namespace zzz::io;
using namespace zzz::common;

namespace zzz::core
{
	class SceneData final : public ISerializable, public IFileBlockLoggable
	{
	public:
		SceneData() = default;
		SceneData(std::string name, Guid sceneScriptGuid)
			: name(std::move(name))
			, sceneScriptGuid(sceneScriptGuid)
		{}

		std::string name;
		Guid sceneScriptGuid;

		void LogFileBlock() const override
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			DOut("           [SceneData] Имя сцены: '{}' | Скрипт сцены GUID: {}", name, sceneScriptGuid.ToString());
#endif
		}

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			return serializer.Serialize(buffer, name)
				.and_then([&]() { return serializer.Serialize(buffer, sceneScriptGuid); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			return serializer.Deserialize(buffer, offset, name)
				.and_then([&]() { return serializer.Deserialize(buffer, offset, sceneScriptGuid); });
		}
	};
}
