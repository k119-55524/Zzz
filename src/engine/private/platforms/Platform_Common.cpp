
#include "Platform.h"

using namespace zzz::engine;

Platform::Platform(const Path& path, std::shared_ptr<NativeAppData> nativeData) :
	m_NativeData(nativeData)
{
	Initialize(path);
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

void Platform::Initialize(const Path& path)
{
	m_ConfigManager = safe_make_shared<ConfigManager>(path);
	InitializePlatformSpecific();
}