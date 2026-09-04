#pragma once

#include <string>
#include <string_view>
#include <vector>
#include "core/utils/Guid.h"
#include "core/serialize/Serializer.h"
#include "core/io/package/GameObjectData.h"
#include "engine/gapi/clear_config/ClearConfig.h"

namespace zzz::core
{
	class SceneData final : public ISerializable
	{
	public:
		SceneData() = default;
		explicit SceneData(
			std::vector<Guid> sceneScriptGuids,
			zzz::engine::ClearConfig clearConfig = {},
			std::vector<GameObjectData> gameObjects = {})
			: sceneScriptGuids(std::move(sceneScriptGuids))
			, clearConfig(std::move(clearConfig))
			, gameObjects(std::move(gameObjects))
		{}

		[[nodiscard]] const std::vector<Guid>& GetSceneScriptGuids() const noexcept { return sceneScriptGuids; }

		[[nodiscard]] const zzz::engine::ClearConfig& GetClearConfig() const noexcept { return clearConfig; }
		void SetClearConfig(const zzz::engine::ClearConfig& config) noexcept { clearConfig = config; }

		[[nodiscard]] const std::vector<GameObjectData>& GetGameObjects() const noexcept { return gameObjects; }
		[[nodiscard]] std::vector<GameObjectData>& GetGameObjects() noexcept { return gameObjects; }
		void SetGameObjects(std::vector<GameObjectData> objs) noexcept { gameObjects = std::move(objs); }

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut(::zzz::core::Assets, "{}[SceneData]", indentation);
			DOut(::zzz::core::Assets, "{}sceneScriptGuids({})", nestedIndentation, sceneScriptGuids.size());
			for (zU32 i = 0; i < sceneScriptGuids.size(); ++i)
			{
				DOut(::zzz::core::Assets, "{}  sceneScriptGuid #{}: {}", nestedIndentation, i, sceneScriptGuids[i].ToString());
			}
			DOut(::zzz::core::Assets, "{}gameObjects({})", nestedIndentation, gameObjects.size());
			for (zU32 i = 0; i < gameObjects.size(); ++i)
			{
				DOut(::zzz::core::Assets, "{}  gameObject #{}: {} [{}]", nestedIndentation, i, gameObjects[i].GetName(), gameObjects[i].GetGuid().ToString());
			}
			clearConfig.LogFileBlock(nestedIndentation);
		}

	private:
		std::vector<Guid> sceneScriptGuids;
		zzz::engine::ClearConfig clearConfig;
		std::vector<GameObjectData> gameObjects;

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			const zU32 scriptsCount = static_cast<zU32>(sceneScriptGuids.size());

			return serializer.Serialize(buffer, scriptsCount)
				.and_then([&]() -> std::expected<void, std::string> {
					for (const auto& scriptGuid : sceneScriptGuids)
					{
						auto res = serializer.Serialize(buffer, scriptGuid);
						if (!res) return res;
					}
					return {};
				})
				.and_then([&]() { return serializer.Serialize(buffer, clearConfig); })
				.and_then([&]() -> std::expected<void, std::string> {
					const zU32 objectsCount = static_cast<zU32>(gameObjects.size());
					auto res = serializer.Serialize(buffer, objectsCount);
					if (!res) return res;

					for (const auto& objData : gameObjects)
					{
						res = serializer.Serialize(buffer, objData);
						if (!res) return res;
					}
					return {};
				});
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			zU32 scriptsCount = 0;

			auto res = serializer.Deserialize(buffer, offset, scriptsCount)
				.and_then([&]() -> std::expected<void, std::string> {
					sceneScriptGuids.clear();
					sceneScriptGuids.reserve(scriptsCount);
					for (zU32 i = 0; i < scriptsCount; ++i)
					{
						Guid scriptGuid{};
						auto r = serializer.Deserialize(buffer, offset, scriptGuid);
						if (!r) return r;
						sceneScriptGuids.push_back(scriptGuid);
					}
					return {};
				})
				.and_then([&]() { return serializer.Deserialize(buffer, offset, clearConfig); });

			if (!res)
			{
				return res;
			}

			// Обратная совместимость (Правило 31): если буфер кончился (старый формат SceneData), объектов 0
			gameObjects.clear();
			if (offset < buffer.size())
			{
				zU32 objectsCount = 0;
				res = serializer.Deserialize(buffer, offset, objectsCount);
				if (!res)
				{
					return res;
				}

				gameObjects.reserve(objectsCount);
				for (zU32 i = 0; i < objectsCount; ++i)
				{
					GameObjectData objData{};
					res = serializer.Deserialize(buffer, offset, objData);
					if (!res)
					{
						return res;
					}
					gameObjects.push_back(std::move(objData));
				}
			}

			return {};
		}
	};
}
