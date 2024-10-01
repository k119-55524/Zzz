#include "pch.h"
#include "AndroidMainAppLoop.h"

using namespace Zzz::Platforms;

#ifdef __ANDROID__

AndroidMainAppLoop::AndroidMainAppLoop(unique_ptr<IWinApp> _win, unique_ptr<IGAPI> _gapi) :
	IMainAppLoop(move(_win), move(_gapi))
{
}

AndroidMainAppLoop::~AndroidMainAppLoop()
{
}

void AndroidMainAppLoop::Run()
{
	ALooper_pollAll(-1, nullptr, nullptr, nullptr);  // -1 значит "ждать бесконечно"
	// Обработка событий
}

#endif // __ANDROID__