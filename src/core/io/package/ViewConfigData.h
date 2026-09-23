#pragma once

#include <span>
#include <vector>
#include <string_view>

#include "core/utils/Guid.h"
#include "core/logger/logger.h"
#include "core/Serialize/Serializer.h"
#include "platforms/start_view/ViewPlatformConfig.h"

namespace zzz::core
{
	/**
	 * @class ViewConfigData
	 * @brief Базовый класс данных конфигурации вида/окна ( PrimaryViewData, ChildViewData, IndependentViewData ).
	 */
	class ViewConfigData : public ISerializable
	{
	public:
		ViewConfigData() = default;
		ViewConfigData(Guid viewGuid, Guid sceneGuid, std::vector<Guid> uiScriptGuids, ViewPlatformData platformData = {})
			: m_ViewGuid(viewGuid)
			, m_SceneGuid(sceneGuid)
			, m_UiScriptGuids(std::move(uiScriptGuids))
			, m_PlatformData(std::move(platformData))
		{}
		~ViewConfigData() override = default;

		[[nodiscard]] const Guid& GetViewGuid() const noexcept { return m_ViewGuid; }
		[[nodiscard]] const Guid& GetSceneGuid() const noexcept { return m_SceneGuid; }
		[[nodiscard]] std::span<const Guid> GetUiScriptGuids() const noexcept { return m_UiScriptGuids; }
		[[nodiscard]] const ViewPlatformData& GetPlatformData() const noexcept { return m_PlatformData; }

	protected:
		Guid m_ViewGuid;
		Guid m_SceneGuid;
		std::vector<Guid> m_UiScriptGuids;
		ViewPlatformData m_PlatformData;

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
				.and_then([&]() { return serializer.Deserialize(buffer, offset, scriptsCount); })
				.and_then([&]() -> std::expected<void, std::string> {
					if (auto v = Serializer::ValidateElementCount(buffer, offset, scriptsCount, Guid::BinarySize()); !v)
						return v;

					m_UiScriptGuids.clear();
					m_UiScriptGuids.reserve(scriptsCount);
					for (zU32 i = 0; i < scriptsCount; ++i)
					{
						Guid scriptGuid;
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
