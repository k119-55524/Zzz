#include "pch.h"
#include "FactoryInit.h"
#include "../Platforms/GAPI/DirectX12API.h"
#include "../Platforms/WinApp/WinAppMSWindows.h"
#include "../Platforms/MainLoop/iOSMainAppLoop.h"
#include "../Platforms/MainLoop/MacOSMainAppLoop.h"
#include "../Platforms/MainLoop/WindowsMainAppLoop.h"

using namespace Zzz;

unique_ptr<IWinApp> FactoryInit::GetAplicationWindows(function<void(const zSize& size, e_TypeWinAppResize resType)> _resizeWindows)
{
	unique_ptr<IWinApp> appWin;

#ifdef _WINDOWS
	appWin = make_unique<WinAppMSWindows>(_resizeWindows);
#elif defined(_MACOS)
	// Не реализовано
	throw runtime_error(">>>>> [InitializationFactory::GetAplicationWindows()]. There is no implementation for the MacOS platform.");
#else
	throw runtime_error(">>>>> [InitializationFactory::GetAplicationWindows()]. Unknown platform.");
#endif

	if (appWin == nullptr)
		throw runtime_error(">>>>> [InitializationFactory::GetAplicationWindows()]. Failed to allocate memory for appWin.");

	return appWin;
}

unique_ptr<IGAPI> FactoryInit::GetGraphicsAPI(const shared_ptr<IWinApp> appWin)
{
	if (appWin == nullptr)
		throw invalid_argument(">>>>> [FactoryInit::GetGraphicsAPI( ... )]. appWin == nullptr.");

	unique_ptr<IGAPI> gapi;

#ifdef _GAPI_DX12
	gapi = make_unique<DirectX12API>(appWin);
#elif defined(_MACOS)
	// Не реализовано
	throw runtime_error(">>>>> [InitializationFactory::GetGraphicsAPI()]. There is no implementation for the MacOS platform.");
#else
	throw runtime_error(">>>>> [InitializationFactory::GetGraphicsAPI()]. Unknown platform.");
#endif

	if (gapi == nullptr)
		throw runtime_error(">>>>> [InitializationFactory::GetGraphicsAPI()]. Failed to allocate memory for gapi.");

	return gapi;
}

unique_ptr<IMainAppLoop> FactoryInit::GetMainAppLoop(function<void()> _updateSystem)
{
	unique_ptr<IMainAppLoop> mainAppLoop;

#ifdef _WINDOWS
	mainAppLoop = make_unique<WindowsMainAppLoop>(_updateSystem);
#elif defined(__APPLE__) && !TARGET_OS_IOS
	mainAppLoop = make_unique<MacOSMainAppLoop>(_updateSystem);
#elif defined(__APPLE__) && TARGET_OS_IOS
	mainAppLoop = make_unique<iOSMainAppLoop>((_updateSystem));
#elif defined(__ANDROID__)
	mainAppLoop = make_unique<AndroidMainAppLoop>((_updateSystem));
#elif defined(__linux__)
	mainAppLoop = make_unique<LinuxMainAppLoop>((_updateSystem));
#else
	throw runtime_error(">>>>> [InitializationFactory::GetMainAppLoop(win, gapi)]. Unknown platform.");
#endif

	if (mainAppLoop == nullptr)
		throw runtime_error(">>>>> [InitializationFactory::GetMainAppLoop(win, gapi)]. Failed to allocate memory for mainAppLoop.");

	return mainAppLoop;
}
