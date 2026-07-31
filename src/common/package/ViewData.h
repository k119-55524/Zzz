#pragma once

#include <string>
#include <vector>
#include <common/guid.h>
#include <common/templates/Size2D.h>
#include <common/serialize/Serializer.h>

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

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			auto res = serializer.Serialize(buffer, name);
			if (!res)
				return res;

			res = serializer.Serialize(buffer, size);
			if (!res)
				return res;

			res = serializer.Serialize(buffer, sceneGuid);
			if (!res)
				return res;

			const zU32 scriptsCount = static_cast<zU32>(uiScriptGuids.size());
			res = serializer.Serialize(buffer, scriptsCount);
			if (!res)
				return res;

			for (const auto& scriptGuid : uiScriptGuids)
			{
				res = serializer.Serialize(buffer, scriptGuid);
				if (!res)
					return res;
			}

			return {};
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			auto res = serializer.Deserialize(buffer, offset, name);
			if (!res)
				return res;

			res = serializer.Deserialize(buffer, offset, size);
			if (!res)
				return res;

			res = serializer.Deserialize(buffer, offset, sceneGuid);
			if (!res)
				return res;

			zU32 scriptsCount = 0;
			res = serializer.Deserialize(buffer, offset, scriptsCount);
			if (!res)
				return res;

			uiScriptGuids.clear();
			uiScriptGuids.reserve(scriptsCount);
			for (zU32 i = 0; i < scriptsCount; ++i)
			{
				Guid scriptGuid{};
				res = serializer.Deserialize(buffer, offset, scriptGuid);
				if (!res)
					return res;

				uiScriptGuids.push_back(scriptGuid);
			}

			return {};
		}
	};
}
