#include "engine/gapi/IGAPI.h"

namespace zzz::engine
{
	IGAPI::IGAPI(std::shared_ptr<UserSettingsManager> userSettings, eGAPIType type)
		: m_UserSettings(std::move(userSettings))
		, m_GAPIType(type)
		, m_InitState(eInitState::NotInitialized)
	{
		ensure(m_UserSettings != nullptr, "UserSettingsManager cannot be null");
	}

	std::expected<void, std::string> IGAPI::Initialize()
	{
		if (m_InitState != eInitState::NotInitialized)
		{
			return std::unexpected("GAPI is already initialized or in an invalid state.");
		}

		m_InitState = eInitState::Initializing;

		auto res = Init();
		if (!res)
		{
			m_InitState = eInitState::NotInitialized;
			return res;
		}

		m_InitState = eInitState::Initialized;
		return {};
	}
}
