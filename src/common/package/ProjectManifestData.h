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
			return serializer.Serialize(buffer, gameScriptGuid)
				.and_then([&]() {
					const zU32 scenesCount = static_cast<zU32>(sceneGuids.size());
					return serializer.Serialize(buffer, scenesCount);
				})
				.and_then([&]() -> std::expected<void, std::string> {
					for (const auto& scGuid : sceneGuids)
					{
						auto res = serializer.Serialize(buffer, scGuid);
						if (!res) return res;
					}
					return {};
				})
				.and_then([&]() {
					const zU32 viewsCount = static_cast<zU32>(viewGuids.size());
					return serializer.Serialize(buffer, viewsCount);
				})
				.and_then([&]() -> std::expected<void, std::string> {
					for (const auto& vGuid : viewGuids)
					{
						auto res = serializer.Serialize(buffer, vGuid);
						if (!res) return res;
					}
					return {};
				});
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			zU32 scenesCount = 0;
			zU32 viewsCount = 0;

			return serializer.Deserialize(buffer, offset, gameScriptGuid)
				.and_then([&]() {
					return serializer.Deserialize(buffer, offset, scenesCount);
				})
				.and_then([&]() -> std::expected<void, std::string> {
					sceneGuids.clear();
					sceneGuids.reserve(scenesCount);
					for (zU32 i = 0; i < scenesCount; ++i)
					{
						Guid scGuid{};
						auto res = serializer.Deserialize(buffer, offset, scGuid);
						if (!res) return res;
						sceneGuids.push_back(scGuid);
					}
					return {};
				})
				.and_then([&]() {
					return serializer.Deserialize(buffer, offset, viewsCount);
				})
				.and_then([&]() -> std::expected<void, std::string> {
					viewGuids.clear();
					viewGuids.reserve(viewsCount);
					for (zU32 i = 0; i < viewsCount; ++i)
					{
						Guid vGuid{};
						auto res = serializer.Deserialize(buffer, offset, vGuid);
						if (!res) return res;
						viewGuids.push_back(vGuid);
					}
					return {};
				});
		}
	};
}
