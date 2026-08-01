#pragma once

#include <vector>
#include <core/Guid.h>
#include <core/Serialize/Serializer.h>

using namespace zzz::io;
using namespace zzz::common;

namespace zzz::core
{
	class ProjectManifestData final : public ISerializable
	{
	public:
		ProjectManifestData() = default;
		ProjectManifestData(std::vector<Guid> gameScriptGuids, std::vector<Guid> sceneGuids, std::vector<Guid> viewGuids)
			: gameScriptGuids(std::move(gameScriptGuids))
			, sceneGuids(std::move(sceneGuids))
			, viewGuids(std::move(viewGuids))
		{}

		std::vector<Guid> gameScriptGuids;
		std::vector<Guid> sceneGuids;
		std::vector<Guid> viewGuids;

		void LogFileBlock() const
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			DOut("           [ProjectManifest] Зарегистрировано глобальных скриптов (GameScripts): {}", gameScriptGuids.size());
			for (zU32 i = 0; i < gameScriptGuids.size(); ++i)
			{
				DOut("             GameScript #{}: {}", i, gameScriptGuids[i].ToString());
			}

			DOut("           [ProjectManifest] Зарегистрировано сцен: {}", sceneGuids.size());
			for (zU32 i = 0; i < sceneGuids.size(); ++i)
			{
				DOut("             Scene #{}: {}", i, sceneGuids[i].ToString());
			}

			DOut("           [ProjectManifest] Зарегистрировано видов (views): {}", viewGuids.size());
			for (zU32 i = 0; i < viewGuids.size(); ++i)
			{
				DOut("             View #{}: {}", i, viewGuids[i].ToString());
			}
#endif
		}

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			const zU32 scriptsCount = static_cast<zU32>(gameScriptGuids.size());

			return serializer.Serialize(buffer, scriptsCount)
				.and_then([&]() -> std::expected<void, std::string> {
					for (const auto& gsGuid : gameScriptGuids)
					{
						auto res = serializer.Serialize(buffer, gsGuid);
						if (!res) return res;
					}
					return {};
				})
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
			zU32 scriptsCount = 0;
			zU32 scenesCount = 0;
			zU32 viewsCount = 0;

			return serializer.Deserialize(buffer, offset, scriptsCount)
				.and_then([&]() -> std::expected<void, std::string> {
					gameScriptGuids.clear();
					gameScriptGuids.reserve(scriptsCount);
					for (zU32 i = 0; i < scriptsCount; ++i)
					{
						Guid gsGuid{};
						auto res = serializer.Deserialize(buffer, offset, gsGuid);
						if (!res) return res;
						gameScriptGuids.push_back(gsGuid);
					}
					return {};
				})
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
