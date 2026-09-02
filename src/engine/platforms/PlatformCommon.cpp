#include "Platform.h"
#if defined(Z_WINDOWS)
#include "monitor/MonitorProviderMSWin.h"
#elif defined(Z_LINUX)
#include "monitor/MonitorProviderLinux.h"
#elif defined(Z_MACOS)
#include "monitor/MonitorProviderMacOS.h"
#elif defined(Z_MOBILE)
#include "monitor/MonitorProviderMobile.h"
#else
#error ">>>>> PlatformCommon: Unsupported platform for IMonitorProvider."
#endif

#include "hardware/HardwareManager.h"

using namespace zzz::engine;

Platform::Platform(std::shared_ptr<NativeAppData> nativeData, const ProjectPlatformData& platformData) :
	m_NativeData(nativeData),
	m_PlatformData(platformData),
	m_MonitorProvider(
#if defined(Z_WINDOWS)
		std::make_shared<MonitorProviderMSWin>()
#elif defined(Z_LINUX)
		std::make_shared<MonitorProviderLinux>()
#elif defined(Z_MACOS)
		std::make_shared<MonitorProviderMacOS>()
#elif defined(Z_MOBILE)
		std::make_shared<MonitorProviderMobile>()
#else
#error ">>>>> PlatformCommon: Unsupported platform for IMonitorProvider."
#endif
	),
	m_HardwareManager(safe_make_unique<HardwareManager>(*m_MonitorProvider))
{
	Initialize();
}

Platform::~Platform()
{
	ShutdownPlatformSpecific();
}

const HardwareState& Platform::GetHardwareState() const noexcept
{
	return m_HardwareManager->GetHardwareState();
}

const IMonitorProvider& Platform::GetMonitorProvider() const noexcept
{
	return *m_MonitorProvider;
}

void Platform::Initialize()
{
	InitializePlatformSpecific();
}
