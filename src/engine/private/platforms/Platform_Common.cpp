
#include "Platform.h"

using namespace zzz::engine;

Platform::Platform(const std::string_view appName, std::shared_ptr<NativeAppData> nativeData) :
	m_AppName(appName),
	m_NativeData(nativeData),
	m_Path(appName, nativeData)
{
	ensure(m_AppName.empty() == false, "Имя приложения не должно быть пустым.");

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
	m_PackageManager = safe_make_shared<PackageManager>(m_Path);
}
