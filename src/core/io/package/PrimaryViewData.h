#pragma once

#include "ViewConfigData.h"

namespace zzz::core
{
	class PrimaryViewData final : public ViewConfigData
	{
	public:
		PrimaryViewData() = default;
		PrimaryViewData(Guid viewGuid, Guid sceneGuid, std::vector<Guid> uiScriptGuids, ViewPlatformData platformData = {})
			: ViewConfigData(viewGuid, sceneGuid, std::move(uiScriptGuids), std::move(platformData))
		{}

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut(::zzz::core::Assets, "{}[PrimaryViewData]", indentation);
			DOut(::zzz::core::Assets, "{}viewGuid: {}", nestedIndentation, m_ViewGuid.ToString());
			DOut(::zzz::core::Assets, "{}sceneGuid: {}", nestedIndentation, m_SceneGuid.ToString());
			DOut(::zzz::core::Assets, "{}uiScriptGuids({})", nestedIndentation, m_UiScriptGuids.size());
			for (zU32 i = 0; i < m_UiScriptGuids.size(); ++i)
			{
				DOut(::zzz::core::Assets, "{}  uiScriptGuid #{}: {}", nestedIndentation, i, m_UiScriptGuids[i].ToString());
			}
			m_PlatformData.LogFileBlock(nestedIndentation);
		}
	};
}
