#include "pch.h"
#include "iOSMainAppLoop.h"

using namespace Zzz::Platforms;

#if defined(__APPLE__) && TARGET_OS_IOS

iOSMainAppLoop::iOSMainAppLoop(unique_ptr<IWinApp> _win, unique_ptr<IGAPI> _gapi) :
	IMainAppLoop(move(_win), move(_gapi))
{
}

iOSMainAppLoop::~iOSMainAppLoop()
{
}

void iOSMainAppLoop::Run()
{
	UIApplicationMain(CommandLine.argc, CommandLine.unsafeArgv, nil, NSStringFromClass([AppDelegate class]));
}

#endif defined(__APPLE__) && TARGET_OS_IOS