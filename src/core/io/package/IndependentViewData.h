#pragma once

#include "ViewConfigData.h"

namespace zzz::core
{
	class IndependentViewData final : public ViewConfigData
	{
	public:
		IndependentViewData() = default;
		IndependentViewData(Guid viewGuid, Guid sceneGuid, std::vector<Guid> uiScriptGuids, ViewPlatformData platformData = {}, zzz::engine::ViewClearConfig clearConfig = {})
			: ViewConfigData(viewGuid, sceneGuid, std::move(uiScriptGuids), std::move(platformData), std::move(clearConfig))
		{}

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut("{}[IndependentViewData]", indentation);
			DOut("{}viewGuid: {}", nestedIndentation, m_ViewGuid.ToString());
			DOut("{}sceneGuid: {}", nestedIndentation, m_SceneGuid.ToString());
			DOut("{}uiScriptGuids({})", nestedIndentation, m_UiScriptGuids.size());
			for (zU32 i = 0; i < m_UiScriptGuids.size(); ++i)
			{
				DOut("{}  uiScriptGuid #{}: {}", nestedIndentation, i, m_UiScriptGuids[i].ToString());
			}
			m_ClearConfig.LogFileBlock(nestedIndentation);
			m_PlatformData.LogFileBlock(nestedIndentation);
		}
	};
}
