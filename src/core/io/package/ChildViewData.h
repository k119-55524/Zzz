#pragma once

#include <string>
#include <string_view>
#include <vector>
#include "core/utils/Guid.h"
#include "platforms/start_view/ViewPlatformConfig.h"
#include "core/Serialize/Serializer.h"

namespace zzz::core
{
	class ChildViewData final : public ISerializable
	{
	public:
		ChildViewData() = default;
		ChildViewData(Guid viewGuid, Guid sceneGuid, std::vector<Guid> uiScriptGuids, ViewPlatformData platformData = {})
			: m_ViewGuid(viewGuid)
			, m_SceneGuid(sceneGuid)
			, m_UiScriptGuids(std::move(uiScriptGuids))
			, m_PlatformData(std::move(platformData))
		{}

		[[nodiscard]] const Guid& GetViewGuid() const noexcept { return m_ViewGuid; }
		[[nodiscard]] const Guid& GetSceneGuid() const noexcept { return m_SceneGuid; }
		[[nodiscard]] const std::vector<Guid>& GetUiScriptGuids() const noexcept { return m_UiScriptGuids; }
		[[nodiscard]] const ViewPlatformData& GetPlatformData() const noexcept { return m_PlatformData; }

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut("{}[ChildViewData]", indentation);
			DOut("{}viewGuid: {}", nestedIndentation, m_ViewGuid.ToString());
			DOut("{}sceneGuid: {}", nestedIndentation, m_SceneGuid.ToString());
			DOut("{}uiScriptGuids({})", nestedIndentation, m_UiScriptGuids.size());
			for (zU32 i = 0; i < m_UiScriptGuids.size(); ++i)
			{
				DOut("{}  uiScriptGuid #{}: {}", nestedIndentation, i, m_UiScriptGuids[i].ToString());
			}
			m_PlatformData.LogFileBlock(nestedIndentation);
		}

	private:
		Guid m_ViewGuid;
		Guid m_SceneGuid;
		std::vector<Guid> m_UiScriptGuids;
		ViewPlatformData m_PlatformData;

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			return serializer.Serialize(buffer, m_ViewGuid)
				.and_then([&]() { return serializer.Serialize(buffer, m_SceneGuid); })
				.and_then([&]() {
					const zU32 scriptsCount = static_cast<zU32>(m_UiScriptGuids.size());
					return serializer.Serialize(buffer, scriptsCount);
				})
				.and_then([&]() -> std::expected<void, std::string> {
					for (const auto& scriptGuid : m_UiScriptGuids)
					{
						auto res = serializer.Serialize(buffer, scriptGuid);
						if (!res) return res;
					}
					return {};
				})
				.and_then([&]() { return serializer.Serialize(buffer, m_PlatformData); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			zU32 scriptsCount = 0;

			return serializer.Deserialize(buffer, offset, m_ViewGuid)
				.and_then([&]() { return serializer.Deserialize(buffer, offset, m_SceneGuid); })
				.and_then([&]() {
					return serializer.Deserialize(buffer, offset, scriptsCount);
				})
				.and_then([&]() -> std::expected<void, std::string> {
					m_UiScriptGuids.clear();
					m_UiScriptGuids.reserve(scriptsCount);
					for (zU32 i = 0; i < scriptsCount; ++i)
					{
						Guid scriptGuid{};
						auto res = serializer.Deserialize(buffer, offset, scriptGuid);
						if (!res) return res;
						m_UiScriptGuids.push_back(scriptGuid);
					}
					return {};
				})
				.and_then([&]() { return serializer.Deserialize(buffer, offset, m_PlatformData); });
		}
	};
}
