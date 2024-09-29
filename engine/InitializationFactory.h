#pragma once

#include "source/Platforms/GAPI/GAPIBase.h"
#include "source/Platforms/WinApplication/WinAppBase.h"
#include "source/Platforms/MessageBox/MessageBoxBase.h"

using namespace Zzz::Platforms;

namespace Zzz
{
	class InitializationFactory
	{
	public:
		unique_ptr<WinAppBase> GetAplicationWindows();
		unique_ptr<GAPIBase> GetGraphicsAPI(unique_ptr<WinAppBase> win);
		unique_ptr<MessageBoxBase> GetSystemImplementationMessageBox();
	};
}
