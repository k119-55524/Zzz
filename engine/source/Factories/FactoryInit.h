#pragma once

#include "../Platforms/GAPI/IGAPI.h"
#include "../Platforms/MainLoop/IMainAppLoop.h"
#include "../Platforms/WinApplication/IWinApp.h"

using namespace Zzz::Platforms;

namespace Zzz
{
	class FactoryInit
	{
	public:
		unique_ptr<IWinApp> GetAplicationWindows();
		unique_ptr<IGAPI> GetGraphicsAPI();
		unique_ptr<IMainAppLoop> GetMainAppLoop(unique_ptr<IWinApp> win, unique_ptr<IGAPI> gapi);
	};
}
