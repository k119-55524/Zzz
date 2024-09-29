#include "pch.h"
#include "InitializationFactory.h"
#include "source/Platforms/GAPI/DirectX12API.h"
#include "source/Platforms/WinApplication/WinAppMSWindows.h"
#include "source/Platforms/MessageBox/MessageBoxMSWindows.h"

using namespace Zzz;

unique_ptr<WinAppBase> Zzz::InitializationFactory::GetAplicationWindows()
{
	static bool IsGet = false;

	if (IsGet)
		throw exception(">>>>> [InitializationFactory::GetAplicationWindows()]. Can only be called once.");

	IsGet = true;

	unique_ptr<WinAppBase> appWin;

#if defined(_WINDOWS)
	appWin = make_unique<WinAppMSWindows>();
#elif defined(_MACOS)
	// Не реализовано
	throw exception(">>>>> [InitializationFactory::GetAplicationWindows()]. There is no implementation for the MacOS platform.");
#else
	throw exception(">>>>> [InitializationFactory::GetAplicationWindows()]. Unknown platform.");
#endif

	if (appWin == nullptr)
		throw exception(">>>>> [InitializationFactory::GetAplicationWindows()]. appWin == nullptr.");

	return appWin;
}

unique_ptr<MessageBoxBase> InitializationFactory::GetSystemImplementationMessageBox()
{
	static bool IsGet = false;

	if (IsGet)
		throw exception(">>>>> [InitializationFactory::GetSystemImplementationMessageBox()]. Can only be called once.");

	IsGet = true;

	unique_ptr<MessageBoxBase> sysMB;

#if defined(_WINDOWS)
	sysMB = make_unique<MessageBoxMSWindows>();
#elif defined(_MACOS)
	// Не реализовано
	throw exception(">>>>> [InitializationFactory::GetSystemImplementationMessageBox()]. There is no implementation for the MacOS platform.");
#else
	throw exception(">>>>> [InitializationFactory::GetSystemImplementationMessageBox()]. Unknown platform.");
#endif

	if (sysMB == nullptr)
		throw exception(">>>>> [InitializationFactory::GetSystemImplementationMessageBox()]. sysMB == nullptr.");

	return sysMB;
}


unique_ptr<GAPIBase> InitializationFactory::GetGraphicsAPI(unique_ptr<WinAppBase> win)
{
	static bool IsGet = false;

	if (win == nullptr)
		throw exception(">>>>> [InitializationFactory::GetGraphicsAPI()]. win == nullptr.");

	if (IsGet)
		throw exception(">>>>> [InitializationFactory::GetGraphicsAPI()]. Can only be called once.");

	IsGet = true;

	unique_ptr<GAPIBase> gapi;

#if defined(_GAPI_DX12)
	gapi = make_unique<DirectX12API>(move(win));
#elif defined(_MACOS)
	// Не реализовано
	throw exception(">>>>> [InitializationFactory::GetGraphicsAPI()]. There is no implementation for the MacOS platform.");
#else
	throw exception(">>>>> [InitializationFactory::GetGraphicsAPI()]. Unknown platform.");
#endif

	if (gapi == nullptr)
		throw exception(">>>>> [InitializationFactory::GetGraphicsAPI()]. gapi == nullptr.");

	return gapi;
}