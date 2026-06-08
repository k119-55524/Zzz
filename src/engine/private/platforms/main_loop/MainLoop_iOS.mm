#if defined(Z_IOS)

#include "MainLoop_iOS.h"

using namespace zzz::engine;

MainLoop_iOS::MainLoop_iOS(const std::shared_ptr<IPlatform> platform) :
	IMainLoop(platform)
{
}

void MainLoop_iOS::Run()
{
}

#endif // defined(Z_IOS)
