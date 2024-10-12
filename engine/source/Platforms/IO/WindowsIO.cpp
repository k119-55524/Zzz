#include "pch.h"
#include "WindowsIO.h"

using namespace Zzz::Platforms;

WindowsIO::WindowsIO()
{
	//appPath;
	//appResourcesPath;

	wchar_t path[MAX_PATH];
	GetModuleFileNameW(NULL, path, sizeof(path) / sizeof(wchar_t));

	zStr directory = path;
	appPath = directory.substr(0, directory.find_last_of(L"\\/")) + separator;
	appResourcesPath = appPath + c_AppResourcesFolder + separator;
}

WindowsIO::~WindowsIO()
{
}
