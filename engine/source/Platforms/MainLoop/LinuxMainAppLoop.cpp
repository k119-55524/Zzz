#include "pch.h"
#include "LinuxMainAppLoop.h"

using namespace Zzz::Platforms;

#ifdef __linux__

LinuxMainAppLoop::LinuxMainAppLoop(unique_ptr<IWinApp> _win, unique_ptr<IGAPI> _gapi) :
	IMainAppLoop(move(_win), move(_gapi))
{
}

LinuxMainAppLoop::~LinuxMainAppLoop()
{
}

void LinuxMainAppLoop::Run()
{
	gtk_main();  // Запуск основного цикла GTK
}

#endif // __linux__