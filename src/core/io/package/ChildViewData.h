#pragma once

#include "ViewConfigData.h"

namespace zzz::core
{
	class ChildViewData final : public ViewConfigData
	{
	public:
		ChildViewData() = default;
		ChildViewData(Guid viewGuid, Guid sceneGuid, std::vector<Guid> uiScriptGuids, ViewPlatformData platformData = {})
			: ViewConfigData(viewGuid, sceneGuid, std::move(uiScriptGuids), std::move(platformData))
		{}

		inline void LogFileBlock([[maybe_unused]] std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut(Assets, "{}[ChildViewData]", indentation);
			DOut(Assets, "{}viewGuid: {}", nestedIndentation, m_ViewGuid.ToString());
			DOut(Assets, "{}sceneGuid: {}", nestedIndentation, m_SceneGuid.ToString());
			DOut(Assets, "{}uiScriptGuids({})", nestedIndentation, m_UiScriptGuids.size());
			for (zU32 i = 0; i < m_UiScriptGuids.size(); ++i)
			{
				DOut(Assets, "{}  uiScriptGuid #{}: {}", nestedIndentation, i, m_UiScriptGuids[i].ToString());
			}
			m_PlatformData.LogFileBlock(nestedIndentation);
#endif
		}
	};
}
