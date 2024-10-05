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
		unique_ptr<IGAPI> GetGraphicsAPI(const shared_ptr<IWinApp> appWin);
		unique_ptr<IMainAppLoop> GetMainAppLoop(function<void()> _updateSystem);
		unique_ptr<IWinApp> GetAplicationWindows(function<void(const zSize& size, e_TypeWinAppResize resType)> _resizeWindows);
	};
}
