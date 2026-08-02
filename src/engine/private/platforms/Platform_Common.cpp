
#include "Platform.h"

using namespace zzz::engine;

Platform::Platform(std::shared_ptr<NativeAppData> nativeData) :
	m_NativeData(nativeData)
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
}