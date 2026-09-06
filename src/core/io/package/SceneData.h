#pragma once

#include <string>
#include <string_view>
#include <vector>
#include "core/utils/Guid.h"
#include "core/serialize/Serializer.h"
#include "core/io/package/LayerData.h"
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
			std::vector<LayerData> layers = {},
			eTransitionSource transitionSource = eTransitionSource::UseGlobal,
			SceneTransitionParams transitionParams = {})
			: sceneScriptGuids(std::move(sceneScriptGuids))
			, clearConfig(std::move(clearConfig))
			, layers(std::move(layers))
			, transitionSource(transitionSource)
			, transitionParams(std::move(transitionParams))
		{}

		[[nodiscard]] const std::vector<Guid>& GetSceneScriptGuids() const noexcept { return sceneScriptGuids; }

		[[nodiscard]] const zzz::engine::ClearConfig& GetClearConfig() const noexcept { return clearConfig; }
		void SetClearConfig(const zzz::engine::ClearConfig& config) noexcept { clearConfig = config; }

		[[nodiscard]] const std::vector<LayerData>& GetLayers() const noexcept { return layers; }
		[[nodiscard]] std::vector<LayerData>& GetLayers() noexcept { return layers; }
		void SetLayers(std::vector<LayerData> layersIn) noexcept { layers = std::move(layersIn); }

		[[nodiscard]] eTransitionSource GetTransitionSource() const noexcept { return transitionSource; }
		void SetTransitionSource(eTransitionSource source) noexcept { transitionSource = source; }

		[[nodiscard]] const SceneTransitionParams& GetTransitionParams() const noexcept { return transitionParams; }
		[[nodiscard]] SceneTransitionParams& GetTransitionParams() noexcept { return transitionParams; }
		void SetTransitionParams(const SceneTransitionParams& params) noexcept { transitionParams = params; }

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut(::zzz::core::Assets, "{}[SceneData]", indentation);
			DOut(::zzz::core::Assets, "{}sceneScriptGuids({})", nestedIndentation, sceneScriptGuids.size());
			for (zU32 i = 0; i < sceneScriptGuids.size(); ++i)
			{
				DOut(::zzz::core::Assets, "{}  sceneScriptGuid #{}: {}", nestedIndentation, i, sceneScriptGuids[i].ToString());
			}
			DOut(::zzz::core::Assets, "{}layers({})", nestedIndentation, layers.size());
			for (zU32 i = 0; i < layers.size(); ++i)
			{
				DOut(::zzz::core::Assets, "{}  layer #{}: '{}' [{}], объектов: {}",
					nestedIndentation, i, layers[i].GetName(), ToString(layers[i].GetType()), layers[i].GetObjects().size());
			}
			DOut(::zzz::core::Assets, "{}transitionSource: {}", nestedIndentation, ToString(transitionSource));
			DOut(::zzz::core::Assets, "{}transitionParams: type={}, duration={:.2f}s, blockInput={}, pauseOld={}",
				nestedIndentation, ToString(transitionParams.type), transitionParams.durationSeconds,
				transitionParams.blockUserInput, transitionParams.pauseOldSceneUpdate);
			clearConfig.LogFileBlock(nestedIndentation);
		}

	private:
		std::vector<Guid> sceneScriptGuids;
		zzz::engine::ClearConfig clearConfig;
		std::vector<LayerData> layers;
		eTransitionSource transitionSource{ eTransitionSource::UseGlobal };
		SceneTransitionParams transitionParams{};

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
					const zU32 layersCount = static_cast<zU32>(layers.size());
					auto res = serializer.Serialize(buffer, layersCount);
					if (!res) return res;

					for (const auto& layerData : layers)
					{
						res = serializer.Serialize(buffer, layerData);
						if (!res) return res;
					}
					return {};
				})
				.and_then([&]() { return serializer.Serialize(buffer, transitionSource); })
				.and_then([&]() { return serializer.Serialize(buffer, transitionParams); });
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

			// Обратная совместимость (Правило 31): если буфер кончился (старый формат SceneData), слоёв 0
			layers.clear();
			if (offset < buffer.size())
			{
				zU32 layersCount = 0;
				res = serializer.Deserialize(buffer, offset, layersCount);
				if (!res)
				{
					return res;
				}

				layers.reserve(layersCount);
				for (zU32 i = 0; i < layersCount; ++i)
				{
					LayerData layerData{};
					res = serializer.Deserialize(buffer, offset, layerData);
					if (!res)
					{
						return res;
					}
					layers.push_back(std::move(layerData));
				}
			}

			// Обратная совместимость (Правило 31): чтение настроек переходов, если они присутствуют в потоке
			transitionSource = eTransitionSource::UseGlobal;
			transitionParams = SceneTransitionParams{};
			if (offset < buffer.size())
			{
				res = serializer.Deserialize(buffer, offset, transitionSource);
				if (!res)
				{
					return res;
				}

				res = serializer.Deserialize(buffer, offset, transitionParams);
				if (!res)
				{
					return res;
				}
			}

			return {};
		}
	};
}
