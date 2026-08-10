#include "Platform.h"
#include "window/WinMSWindowEditor.h"

using namespace zzz::engine;

void Platform::ShutdownPlatformSpecific()
{
}

void Platform::InitializePlatformSpecific()
{
}

PlatformHardwareState Platform::GatherHardwareState() const
{
	PlatformHardwareState state{};
	return state;
}
