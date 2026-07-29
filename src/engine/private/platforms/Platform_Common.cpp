
#include "Platform.h"

using namespace zzz::engine;

Platform::Platform(const Path& path, std::shared_ptr<NativeAppData> nativeData) :
	m_Path{path},
	m_NativeData(nativeData)
{
	Initialize();
}

Platform::~Platform()
{
	if (m_ConfigManager != nullptr)
	{
		auto res = m_ConfigManager->SaveConfig();
		if (!res)
			DOutCritical("Не удалось сериализовать конфигурацию: {}.", res.error());
	}

	ShutdownPlatformSpecific();
}

void Platform::Initialize()
{
	m_ConfigManager = safe_make_shared<ConfigManager>(m_Path);
	InitializePlatformSpecific();
}