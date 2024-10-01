#include "pch.h"
#include "WinAppMSWindows.h"

using namespace Zzz;
using namespace Zzz::Platforms;

#ifdef _WINDOWS

WinAppMSWindows::WinAppMSWindows()
{
}

WinAppMSWindows::~WinAppMSWindows()
{
}

zResult WinAppMSWindows::Initialize(const s_zEngineInit::WinAppSettings* data)
{
	const wchar_t CLASS_NAME[] = L"Sample Window Class";

	return zResult();
}

#endif // _WINDOWS