#pragma once

#include <vector>
#include <common/guid.h>
#include <common/serialize/Serializer.h>

using namespace zzz::common;

namespace zzz::core
{
	class ProjectManifestData final : public ISerializable
	{
	public:
		ProjectManifestData() = default;
		ProjectManifestData(Guid gameScriptGuid, std::vector<Guid> sceneGuids, std::vector<Guid> viewGuids)
			: gameScriptGuid(gameScriptGuid)
			, sceneGuids(std::move(sceneGuids))
			, viewGuids(std::move(viewGuids))
		{}

		Guid gameScriptGuid;
		std::vector<Guid> sceneGuids;
		std::vector<Guid> viewGuids;

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			auto res = serializer.Serialize(buffer, gameScriptGuid);
			if (!res) return res;

			const zU32 scenesCount = static_cast<zU32>(sceneGuids.size());
			res = serializer.Serialize(buffer, scenesCount);
			if (!res) return res;

			for (const auto& scGuid : sceneGuids)
			{
				res = serializer.Serialize(buffer, scGuid);
				if (!res) return res;
			}

			const zU32 viewsCount = static_cast<zU32>(viewGuids.size());
			res = serializer.Serialize(buffer, viewsCount);
			if (!res) return res;

			for (const auto& vGuid : viewGuids)
			{
				res = serializer.Serialize(buffer, vGuid);
				if (!res) return res;
			}

			return {};
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			auto res = serializer.Deserialize(buffer, offset, gameScriptGuid);
			if (!res) return res;

			zU32 scenesCount = 0;
			res = serializer.Deserialize(buffer, offset, scenesCount);
			if (!res) return res;

			sceneGuids.clear();
			sceneGuids.reserve(scenesCount);
			for (zU32 i = 0; i < scenesCount; ++i)
			{
				Guid scGuid{};
				res = serializer.Deserialize(buffer, offset, scGuid);
				if (!res) return res;
				sceneGuids.push_back(scGuid);
			}

			zU32 viewsCount = 0;
			res = serializer.Deserialize(buffer, offset, viewsCount);
			if (!res) return res;

			viewGuids.clear();
			viewGuids.reserve(viewsCount);
			for (zU32 i = 0; i < viewsCount; ++i)
			{
				Guid vGuid{};
				res = serializer.Deserialize(buffer, offset, vGuid);
				if (!res) return res;
				viewGuids.push_back(vGuid);
			}

			return {};
		}
	};
}
