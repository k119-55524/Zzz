#pragma once

#include <string>
#include <vector>
#include <core/Guid.h>
#include <logger/logger.h>
#include <common/Templates/Size2D.h>
#include <core/Serialize/Serializer.h>
#include "platforms/AppViewPlatformConfig.h"

using namespace zzz::io;
using namespace zzz::common;

namespace zzz::core
{
	class AppViewData final : public ISerializable
	{
	public:
		AppViewData() = default;
		AppViewData(std::string title, Guid sceneGuid, std::vector<Guid> uiScriptGuids, AppViewPlatformData platformData = {})
			: title(std::move(title))
			, sceneGuid(sceneGuid)
			, uiScriptGuids(std::move(uiScriptGuids))
			, platformData(std::move(platformData))
		{}

		[[nodiscard]] const std::string& GetTitle() const noexcept { return title; }
		[[nodiscard]] const Guid& GetSceneGuid() const noexcept { return sceneGuid; }
		[[nodiscard]] const std::vector<Guid>& GetUiScriptGuids() const noexcept { return uiScriptGuids; }
		[[nodiscard]] const AppViewPlatformData& GetPlatformData() const noexcept { return platformData; }

		// Хелпер для получения дефолтного размера (если поддерживается платформой)
		[[nodiscard]] Size2D<zU32> GetDefaultSize() const noexcept
		{
			if constexpr (requires { platformData.GetDefaultSize(); })
				return platformData.GetDefaultSize();
			else
				return Size2D<zU32>{ 1280, 720 };
		}



		// Хелпер для получения флага изменяемости размера
		[[nodiscard]] bool IsResizable() const noexcept
		{
			if constexpr (requires { platformData.IsResizable(); })
				return platformData.IsResizable();
			else
				return true;
		}

		inline void LogFileBlock() const
		{
			DOut("           [AppViewData] title: {}", title);
			DOut("           [AppViewData] sceneGuid: {}", sceneGuid.ToString());
			DOut("           [AppViewData] uiScriptGuids({})", uiScriptGuids.size());
			for (zU32 i = 0; i < uiScriptGuids.size(); ++i)
			{
				DOut("             uiScriptGuid #{}: {}", i, uiScriptGuids[i].ToString());
			}
		}

	private:
		std::string title;
		Guid sceneGuid;
		std::vector<Guid> uiScriptGuids;
		AppViewPlatformData platformData;

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			return serializer.Serialize(buffer, title)
				.and_then([&]() { return serializer.Serialize(buffer, sceneGuid); })
				.and_then([&]() {
					const zU32 scriptsCount = static_cast<zU32>(uiScriptGuids.size());
					return serializer.Serialize(buffer, scriptsCount);
				})
				.and_then([&]() -> std::expected<void, std::string> {
					for (const auto& scriptGuid : uiScriptGuids)
					{
						auto res = serializer.Serialize(buffer, scriptGuid);
						if (!res) return res;
					}
					return {};
				})
				.and_then([&]() { return serializer.Serialize(buffer, platformData); });
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			zU32 scriptsCount = 0;

			return serializer.Deserialize(buffer, offset, title)
				.and_then([&]() { return serializer.Deserialize(buffer, offset, sceneGuid); })
				.and_then([&]() {
					return serializer.Deserialize(buffer, offset, scriptsCount);
				})
				.and_then([&]() -> std::expected<void, std::string> {
					uiScriptGuids.clear();
					uiScriptGuids.reserve(scriptsCount);
					for (zU32 i = 0; i < scriptsCount; ++i)
					{
						Guid scriptGuid{};
						auto res = serializer.Deserialize(buffer, offset, scriptGuid);
						if (!res) return res;
						uiScriptGuids.push_back(scriptGuid);
					}
					return {};
				})
				.and_then([&]() { return serializer.Deserialize(buffer, offset, platformData); });
		}
	};
}
