#include "engine/gapi/IGAPI.h"

namespace zzz::engine
{
	IGAPI::IGAPI(std::shared_ptr<UserSettingsManager> userSettings)
		: m_UserSettings(std::move(userSettings))
	{
		ensure(m_UserSettings != nullptr, "UserSettingsManager cannot be null");
	}
}
