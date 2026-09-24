#pragma once
#include <span>

#include <string>
#include <string_view>
#include <vector>
#include "core/utils/Guid.h"
#include "core/utils/Version.h"
#include "core/Serialize/Serializer.h"
#include "core/enums/ePackage.h"
#include "core/scene/SceneTransitionParams.h"
#include "core/IO/package/platforms/project/ProjectPlatformConfig.h"

namespace zzz::core
{
	struct SceneManifestEntry final : public ISerializable
	{
		std::string name;
		Guid guid;

		SceneManifestEntry() = default;
		SceneManifestEntry(std::string name, const Guid& guid)
			: name(std::move(name)), guid(guid) {}

		[[nodiscard]] const std::string& GetName() const noexcept { return name; }
		[[nodiscard]] const Guid& GetGuid() const noexcept { return guid; }

		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			return serializer.Serialize(buffer, name)
				.and_then([&]() { return serializer.Serialize(buffer, guid); });
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			return serializer.Deserialize(buffer, offset, name)
				.and_then([&]() { return serializer.Deserialize(buffer, offset, guid); });
		}
	};

	class ProjectManifestData final : public ISerializable
	{
	public:
		static constexpr ePackage c_PackageType = ePackage::ProjectManifest;

		ProjectManifestData() = default;
		ProjectManifestData(
			std::vector<Guid> gameScriptGuids,
			std::vector<SceneManifestEntry> scenes,
			std::vector<Guid> viewGuids,
			ProjectPlatformData platformData = {},
			zU32 maxLogQueueSize = c_MaxNetworkLogQueueSize,
			zU16 loggerPort = c_DefaultLoggerPort,
			std::string appName = {},
			std::string companyName = {},
			Version appVersion = {},
			SceneTransitionParams defaultTransitionParams = {})
			: gameScriptGuids(std::move(gameScriptGuids))
			, scenes(std::move(scenes))
			, viewGuids(std::move(viewGuids))
			, platformData(std::move(platformData))
			, maxLogQueueSize(maxLogQueueSize)
			, loggerPort(loggerPort)
			, appName(std::move(appName))
			, companyName(std::move(companyName))
			, appVersion(appVersion)
			, defaultTransitionParams(std::move(defaultTransitionParams))
		{}

		[[nodiscard]] std::span<const SceneManifestEntry> GetScenes() const noexcept { return scenes; }
		[[nodiscard]] std::span<const Guid> GetGameScriptGuids() const noexcept { return gameScriptGuids; }
		[[nodiscard]] std::span<const Guid> GetViewGuids() const noexcept { return viewGuids; }
		[[nodiscard]] const ProjectPlatformData& GetPlatformData() const noexcept { return platformData; }
		[[nodiscard]] zU32 GetMaxLogQueueSize() const noexcept { return maxLogQueueSize; }
		[[nodiscard]] zU16 GetLoggerPort() const noexcept { return loggerPort; }
		[[nodiscard]] const std::string& GetAppName() const noexcept { return appName; }
		[[nodiscard]] const std::string& GetCompanyName() const noexcept { return companyName; }
		[[nodiscard]] const Version& GetAppVersion() const noexcept { return appVersion; }
		[[nodiscard]] const SceneTransitionParams& GetDefaultTransitionParams() const noexcept { return defaultTransitionParams; }

		void LogFileBlock([[maybe_unused]] std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut(Assets, "{}[ProjectManifestData]", indentation);
			DOut(Assets, "{}appName: {}", nestedIndentation, appName);
			DOut(Assets, "{}companyName: {}", nestedIndentation, companyName);
			DOut(Assets, "{}appVersion: {}", nestedIndentation, appVersion.ToString());
			DOut(Assets, "{}gameScriptGuids({})", nestedIndentation, gameScriptGuids.size());
			for (zU32 i = 0; i < gameScriptGuids.size(); ++i)
			{
				DOut(Assets, "{}  gameScriptGuid #{}: {}", nestedIndentation, i, gameScriptGuids[i].ToString());
			}

			DOut(Assets, "{}scenes({})", nestedIndentation, scenes.size());
			for (zU32 i = 0; i < scenes.size(); ++i)
			{
				DOut(Assets, "{}  scene #{}: '{}' [{}]", nestedIndentation, i, scenes[i].GetName(), scenes[i].GetGuid().ToString());
			}

			DOut(Assets, "{}viewGuids({})", nestedIndentation, viewGuids.size());
			for (zU32 i = 0; i < viewGuids.size(); ++i)
			{
				DOut(Assets, "{}  viewGuid #{}: {}", nestedIndentation, i, viewGuids[i].ToString());
			}

			DOut(Assets, "{}maxLogQueueSize: {}", nestedIndentation, maxLogQueueSize);
			DOut(Assets, "{}loggerPort: {}", nestedIndentation, loggerPort);
			DOut(Assets, "{}defaultTransitionParams: type={}, duration={:.2f}s, blockInput={}, pauseOld={}",
				nestedIndentation, ToString(defaultTransitionParams.type), defaultTransitionParams.durationSeconds,
				defaultTransitionParams.blockUserInput, defaultTransitionParams.pauseOldSceneUpdate);

			platformData.LogFileBlock(nestedIndentation);
#endif
		}

	private:
		std::vector<Guid> gameScriptGuids;
		std::vector<SceneManifestEntry> scenes;
		std::vector<Guid> viewGuids;
		ProjectPlatformData platformData;
		zU32 maxLogQueueSize{ c_MaxNetworkLogQueueSize };
		zU16 loggerPort{ c_DefaultLoggerPort };
		std::string appName;
		std::string companyName;
		Version appVersion;
		SceneTransitionParams defaultTransitionParams{};

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
					const zU32 scenesCount = static_cast<zU32>(scenes.size());
					return serializer.Serialize(buffer, scenesCount);
				})
				.and_then([&]() -> std::expected<void, std::string> {
					for (const auto& sc : scenes)
					{
						auto res = sc.Serialize(buffer, serializer);
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
				})
				.and_then([&]() {
					return serializer.Serialize(buffer, maxLogQueueSize);
				})
				.and_then([&]() {
					return serializer.Serialize(buffer, loggerPort);
				})
				.and_then([&]() {
					return serializer.Serialize(buffer, platformData);
				})
				.and_then([&]() {
					return serializer.Serialize(buffer, appName);
				})
				.and_then([&]() {
					return serializer.Serialize(buffer, companyName);
				})
				.and_then([&]() {
					return serializer.Serialize(buffer, appVersion);
				})
				.and_then([&]() {
					return serializer.Serialize(buffer, defaultTransitionParams);
				});
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			zU32 scriptsCount = 0;
			zU32 scenesCount = 0;
			zU32 viewsCount = 0;

			return serializer.Deserialize(buffer, offset, scriptsCount)
				.and_then([&]() -> std::expected<void, std::string>
				{
					if (auto v = Serializer::ValidateElementCount(buffer, offset, scriptsCount, Guid::BinarySize()); !v)
						return v;

					gameScriptGuids.clear();
					gameScriptGuids.reserve(scriptsCount);
					for (zU32 i = 0; i < scriptsCount; ++i)
					{
						Guid gsGuid{};
						auto res = serializer.Deserialize(buffer, offset, gsGuid);
						if (!res)
							return res;

						gameScriptGuids.push_back(gsGuid);
					}

					return {};
				})
				.and_then([&]()
				{
					return serializer.Deserialize(buffer, offset, scenesCount);
				})
				.and_then([&]() -> std::expected<void, std::string>
				{
					// SceneManifestEntry: длина имени (zU32) + Guid
					if (auto v = Serializer::ValidateElementCount(buffer, offset, scenesCount, sizeof(zU32) + Guid::BinarySize()); !v)
						return v;

					scenes.clear();
					scenes.reserve(scenesCount);
					for (zU32 i = 0; i < scenesCount; ++i)
					{
						SceneManifestEntry sc{};
						auto res = serializer.Deserialize(buffer, offset, sc);
						if (!res)
							return res;

						scenes.push_back(std::move(sc));
					}

					return {};
				})
				.and_then([&]()
				{
					return serializer.Deserialize(buffer, offset, viewsCount);
				})
				.and_then([&]() -> std::expected<void, std::string>
				{
					if (auto v = Serializer::ValidateElementCount(buffer, offset, viewsCount, Guid::BinarySize()); !v)
						return v;

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
				})
				.and_then([&]()
				{
					return serializer.Deserialize(buffer, offset, maxLogQueueSize);
				})
				.and_then([&]()
				{
					return serializer.Deserialize(buffer, offset, loggerPort);
				})
				.and_then([&]()
				{
					return serializer.Deserialize(buffer, offset, platformData);
				})
				.and_then([&]()
				{
					return serializer.Deserialize(buffer, offset, appName);
				})
				.and_then([&]()
				{
					return serializer.Deserialize(buffer, offset, companyName);
				})
				.and_then([&]()
				{
					return serializer.Deserialize(buffer, offset, appVersion);
				})
				.and_then([&]() -> std::expected<void, std::string>
				{
					// Обратная совместимость (Правило 31): если буфер закончился, дефолтные параметры перехода
					defaultTransitionParams = SceneTransitionParams{};
					if (offset < buffer.size())
					{
						return serializer.Deserialize(buffer, offset, defaultTransitionParams);
					}
					return {};
				});
		}
	};
}
