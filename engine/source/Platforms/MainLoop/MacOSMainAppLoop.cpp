#include "pch.h"
#include "MacOSMainAppLoop.h"

using namespace Zzz::Platforms;

#if defined(__APPLE__) && !TARGET_OS_IOS

MacOSMainAppLoop::MacOSMainAppLoop(unique_ptr<IWinApp> _win, unique_ptr<IGAPI> _gapi) :
	IMainAppLoop(move(_win), move(_gapi))
{
}

MacOSMainAppLoop::~MacOSMainAppLoop()
{
}

void MacOSMainAppLoop::Run()
{
	[[NSRunLoop mainRunLoop]run];  // Запускаем основной цикл на macOS
}

#endif // defined(__APPLE__) && !TARGET_OS_IOS
