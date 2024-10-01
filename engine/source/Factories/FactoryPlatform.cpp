#include "pch.h"
#include "FactoryPlatform.h"
#include "../Platforms/MessageBox/MB_MSWin.h"

using namespace Zzz;

unique_ptr<ISysMB> FactoryPlatform::GetSystemImplementationMessageBox()
{
	static bool IsGet = false;

	if (IsGet)
		throw runtime_error(">>>>> [FactoryPlatform::GetSystemImplementationMessageBox()]. Can only be called once.");

	IsGet = true;

	unique_ptr<ISysMB> sysMB;

#ifdef _WINDOWS
	sysMB = make_unique<MB_MSWin>();
#elif defined(_MACOS)
	// Не реализовано
	throw runtime_error(">>>>> [FactoryPlatform::GetSystemImplementationMessageBox()]. There is no implementation for the MacOS platform.");
#else
	throw runtime_error(">>>>> [FactoryPlatform::GetSystemImplementationMessageBox()]. Unknown platform.");
#endif

	if (sysMB == nullptr)
		throw runtime_error(">>>>> [FactoryPlatform::GetSystemImplementationMessageBox()]. sysMB == nullptr.");

	return sysMB;
}