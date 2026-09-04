#pragma once

#include <string>
#include <string_view>
#include <vector>
#include "core/utils/Guid.h"
#include "core/utils/Version.h"
#include "core/Serialize/Serializer.h"
#include "core/scene/transition/SceneTransitionParams.h"
#include "core/IO/package/platforms/project/ProjectPlatformConfig.h"

namespace zzz::core
{
	class ProjectManifestData final : public ISerializable
	{
	public:
		ProjectManifestData() = default;
		ProjectManifestData(std::vector<Guid> gameScriptGuids, std::vector<Guid> sceneGuids, std::vector<Guid> viewGuids, ProjectPlatformData platformData = {}, zU32 maxLogQueueSize = c_MaxNetworkLogQueueSize, zU16 loggerPort = c_DefaultLoggerPort, std::string appName = {}, std::string companyName = {}, Version appVersion = {}, SceneTransitionParams defaultTransitionParams = {})
			: gameScriptGuids(std::move(gameScriptGuids))
			, sceneGuids(std::move(sceneGuids))
			, viewGuids(std::move(viewGuids))
			, platformData(std::move(platformData))
			, maxLogQueueSize(maxLogQueueSize)
			, loggerPort(loggerPort)
			, appName(std::move(appName))
			, companyName(std::move(companyName))
			, appVersion(appVersion)
			, defaultTransitionParams(std::move(defaultTransitionParams))
		{}

		[[nodiscard]] const std::vector<Guid>& GetGameScriptGuids() const noexcept { return gameScriptGuids; }
		[[nodiscard]] const std::vector<Guid>& GetSceneGuids() const noexcept { return sceneGuids; }
		[[nodiscard]] const std::vector<Guid>& GetViewGuids() const noexcept { return viewGuids; }
		[[nodiscard]] const ProjectPlatformData& GetPlatformData() const noexcept { return platformData; }
		[[nodiscard]] zU32 GetMaxLogQueueSize() const noexcept { return maxLogQueueSize; }
		[[nodiscard]] zU16 GetLoggerPort() const noexcept { return loggerPort; }
		[[nodiscard]] const std::string& GetAppName() const noexcept { return appName; }
		[[nodiscard]] const std::string& GetCompanyName() const noexcept { return companyName; }
		[[nodiscard]] const Version& GetAppVersion() const noexcept { return appVersion; }
		[[nodiscard]] const SceneTransitionParams& GetDefaultTransitionParams() const noexcept { return defaultTransitionParams; }
		void SetDefaultTransitionParams(const SceneTransitionParams& params) noexcept { defaultTransitionParams = params; }

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut(::zzz::core::Assets, "{}[ProjectManifestData]", indentation);
			DOut(::zzz::core::Assets, "{}appName: {}", nestedIndentation, appName);
			DOut(::zzz::core::Assets, "{}companyName: {}", nestedIndentation, companyName);
			DOut(::zzz::core::Assets, "{}appVersion: {}", nestedIndentation, appVersion.ToString());
			DOut(::zzz::core::Assets, "{}gameScriptGuids({})", nestedIndentation, gameScriptGuids.size());
			for (zU32 i = 0; i < gameScriptGuids.size(); ++i)
			{
				DOut(::zzz::core::Assets, "{}  gameScriptGuid #{}: {}", nestedIndentation, i, gameScriptGuids[i].ToString());
			}

			DOut(::zzz::core::Assets, "{}sceneGuids({})", nestedIndentation, sceneGuids.size());
			for (zU32 i = 0; i < sceneGuids.size(); ++i)
			{
				DOut(::zzz::core::Assets, "{}  sceneGuid #{}: {}", nestedIndentation, i, sceneGuids[i].ToString());
			}

			DOut(::zzz::core::Assets, "{}viewGuids({})", nestedIndentation, viewGuids.size());
			for (zU32 i = 0; i < viewGuids.size(); ++i)
			{
				DOut(::zzz::core::Assets, "{}  viewGuid #{}: {}", nestedIndentation, i, viewGuids[i].ToString());
			}

			DOut(::zzz::core::Assets, "{}maxLogQueueSize: {}", nestedIndentation, maxLogQueueSize);
			DOut(::zzz::core::Assets, "{}loggerPort: {}", nestedIndentation, loggerPort);

			platformData.LogFileBlock(nestedIndentation);
		}

	private:
		std::vector<Guid> gameScriptGuids;
		std::vector<Guid> sceneGuids;
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
					sceneGuids.clear();
					sceneGuids.reserve(scenesCount);
					for (zU32 i = 0; i < scenesCount; ++i)
					{
						Guid scGuid{};
						auto res = serializer.Deserialize(buffer, offset, scGuid);
						if (!res)
							return res;

						sceneGuids.push_back(scGuid);
					}

					return {};
				})
				.and_then([&]()
				{
					return serializer.Deserialize(buffer, offset, viewsCount);
				})
				.and_then([&]() -> std::expected<void, std::string>
				{
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
