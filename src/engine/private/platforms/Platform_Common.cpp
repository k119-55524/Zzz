#include <foundation.h>

#include "Platform.h"

using namespace zzz::engine;

Platform::Platform(const std::string_view appName, std::shared_ptr<NativeAppData> nativeData) :
	m_AppName(appName),
	m_NativeData(nativeData),
	m_Path(appName, nativeData)
{
	ensure(m_AppName.empty() == false, "Application name must not be empty.");

	Initialize();
}

Platform::~Platform()
{
	if (m_ConfigManager != nullptr)
	{
		auto res = m_ConfigManager->SaveConfig();
		if (!res)
			DOutCritical("Failed to serialize config: {}.", res.error());
	}

	ShutdownPlatformSpecific();
}

void Platform::Initialize()
{
		m_ConfigManager = zzz::safe_make_shared<ConfigManager>(m_Path);

	InitializePlatformSpecific();
}
