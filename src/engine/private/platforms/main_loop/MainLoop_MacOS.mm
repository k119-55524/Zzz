#if defined(Z_MACOS)

#include "MainLoop_MacOS.h"

using namespace zzz::engine;

MainLoop_MacOS::MainLoop_MacOS(const std::shared_ptr<Platform> platform) :
	IMainLoop(platform)
{
}

void MainLoop_MacOS::Run()
{
}

#endif // defined(Z_MACOS)
