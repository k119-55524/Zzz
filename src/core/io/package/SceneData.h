#pragma once

#include <string>
#include <string_view>
#include <vector>
#include "core/utils/Guid.h"
#include "core/Serialize/Serializer.h"
#include "engine/gapi/clear_config/ClearConfig.h"

namespace zzz::core
{
	class SceneData final : public ISerializable
	{
	public:
		SceneData() = default;
		explicit SceneData(std::vector<Guid> sceneScriptGuids, zzz::engine::ClearConfig clearConfig = {})
			: sceneScriptGuids(std::move(sceneScriptGuids))
			, clearConfig(std::move(clearConfig))
		{}

		[[nodiscard]] const std::vector<Guid>& GetSceneScriptGuids() const noexcept { return sceneScriptGuids; }

		[[nodiscard]] const zzz::engine::ClearConfig& GetClearConfig() const noexcept { return clearConfig; }
		void SetClearConfig(const zzz::engine::ClearConfig& config) noexcept { clearConfig = config; }

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut(::zzz::core::Assets, "{}[SceneData]", indentation);
			DOut(::zzz::core::Assets, "{}sceneScriptGuids({})", nestedIndentation, sceneScriptGuids.size());
			for (zU32 i = 0; i < sceneScriptGuids.size(); ++i)
			{
				DOut(::zzz::core::Assets, "{}  sceneScriptGuid #{}: {}", nestedIndentation, i, sceneScriptGuids[i].ToString());
			}
			clearConfig.LogFileBlock(nestedIndentation);
		}

	private:
		std::vector<Guid> sceneScriptGuids;
		zzz::engine::ClearConfig clearConfig;

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
				.and_then([&]() { return serializer.Serialize(buffer, clearConfig); });
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			zU32 scriptsCount = 0;

			return serializer.Deserialize(buffer, offset, scriptsCount)
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
				})
				.and_then([&]() { return serializer.Deserialize(buffer, offset, clearConfig); });
		}
	};
}
