#include "Platform.h"
#if Z_WINDOWS
#include "monitor/MonitorProviderMSWin.h"
#elif Z_ANDROID || Z_IOS
#include "monitor/MonitorProviderMobile.h"
#else
#include "monitor/MonitorProviderMSWin.h"
#endif

using namespace zzz::engine;

Platform::Platform(std::shared_ptr<NativeAppData> nativeData, const ProjectPlatformData& platformData) :
	m_NativeData(nativeData),
	m_PlatformData(platformData),
	m_MonitorProvider(
#if Z_WINDOWS
		std::make_shared<MonitorProviderMSWin>()
#elif Z_ANDROID || Z_IOS
		std::make_shared<MonitorProviderMobile>()
#else
		std::make_shared<MonitorProviderMSWin>()
#endif
	),
	m_HardwareState(GatherHardwareState())
{
	Initialize();
}

Platform::~Platform()
{
	ShutdownPlatformSpecific();
}

const IMonitorProvider& Platform::GetMonitorProvider() const noexcept
{
	return *m_MonitorProvider;
}

void Platform::Initialize()
{
	InitializePlatformSpecific();
}
