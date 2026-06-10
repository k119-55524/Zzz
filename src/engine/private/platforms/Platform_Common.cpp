#include <foundation.h>

#include "Platform.h"
#include "../factories/EngineFactory.h"

using namespace zzz::engine;

Platform::Platform(std::string_view appName, std::shared_ptr<NativeAppData> nativeData) :
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
	m_Factory = zzz::safe_make_shared<EngineFactory>();
	m_ConfigManager = zzz::safe_make_shared<ConfigManager>(m_Path);

	InitializePlatformSpecific();
}
