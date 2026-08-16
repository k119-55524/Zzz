
#include "Platform.h"

using namespace zzz::engine;

Platform::Platform(std::shared_ptr<NativeAppData> nativeData, const ProjectPlatformData& platformData) :
	m_NativeData(nativeData),
	m_PlatformData(platformData)
{
	Initialize();
}

Platform::~Platform()
{
	ShutdownPlatformSpecific();
}

void Platform::Initialize()
{
	InitializePlatformSpecific();
	m_HardwareState = GatherHardwareState();
}
