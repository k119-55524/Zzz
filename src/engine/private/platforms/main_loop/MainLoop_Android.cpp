#if defined(Z_ANDROID)

#include "MainLoop_Android.h"

using namespace zzz::engine;

MainLoop_Android::MainLoop_Android(const std::shared_ptr<IPlatform> platform) :
	IMainLoop(platform)
{
}

void MainLoop_Android::Run()
{
}

#endif // defined(Z_ANDROID)
