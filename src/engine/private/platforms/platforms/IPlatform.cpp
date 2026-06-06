#include <foundation.h>

#include "IPlatform.h"

using namespace zzz::engine;

IPlatform::IPlatform(std::string_view appName, std::shared_ptr<void> platformData) :
	m_AppName(appName),
	m_PlatformData(platformData),
	m_Path(std::make_shared<Path>(appName, platformData))
{
	ensure(m_AppName.empty() == false, "Application name must not be empty.");
}

IPlatform::~IPlatform()
{
	if (m_ConfigManager != nullptr)
	{
		auto res = m_ConfigManager->SaveConfig();
		if (!res)
			DOutCritical("Failed to serialize config: {}.", res.error());
	}

	m_Path = nullptr;
	m_ConfigManager = nullptr;
}

void IPlatform::Initialize(std::string_view configPath)
{
	m_Path = zzz::safe_make_shared<Path>(m_AppName, m_PlatformData);
	m_ConfigManager = zzz::safe_make_shared<ConfigManager>(m_Path, configPath);

	InitializeImpl();
}