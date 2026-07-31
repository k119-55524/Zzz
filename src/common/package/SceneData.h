#pragma once

#include <string>
#include <common/guid.h>
#include <common/serialize/Serializer.h>

using namespace zzz::common;

namespace zzz::core
{
	class SceneData final : public ISerializable
	{
	public:
		SceneData() = default;
		SceneData(std::string name, Guid sceneScriptGuid)
			: name(std::move(name))
			, sceneScriptGuid(sceneScriptGuid)
		{}

		std::string name;
		Guid sceneScriptGuid;

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			auto res = serializer.Serialize(buffer, name);
			if (!res) return res;

			res = serializer.Serialize(buffer, sceneScriptGuid);
			if (!res) return res;

			return {};
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			auto res = serializer.Deserialize(buffer, offset, name);
			if (!res) return res;

			res = serializer.Deserialize(buffer, offset, sceneScriptGuid);
			if (!res) return res;

			return {};
		}
	};
}
