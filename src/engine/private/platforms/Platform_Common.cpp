#include <foundation.h>

#include "Platform.h"
#include "../factories/EngineFactory.h"

using namespace zzz::engine;

Platform::Platform(std::string_view appName, std::shared_ptr<PlatformNativeData> platformData) :
	m_AppName(appName),
	m_PlatformData(platformData),
	m_Path(std::make_shared<Path>(appName, platformData))
{
	ensure(m_AppName.empty() == false, "Application name must not be empty.");
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
	auto platformConfig = m_Factory->CreatePlatformConfig();
	m_ConfigManager = zzz::safe_make_shared<ConfigManager>(m_Path, std::move(platformConfig));

	InitializePlatformSpecific();
}
