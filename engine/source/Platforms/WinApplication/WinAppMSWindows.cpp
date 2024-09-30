#include "pch.h"
#include "WinAppMSWindows.h"

using namespace Zzz;
using namespace Zzz::Platforms;

#if defined(_WINDOWS)

WinAppMSWindows::WinAppMSWindows()
{
}

zResult WinAppMSWindows::Init(const s_zEngineInit::WinAppSettings* data)
{
	const wchar_t CLASS_NAME[] = L"Sample Window Class";

	return zResult();
}

#endif // defined(_WINDOWS)