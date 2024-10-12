#include "pch.h"
#include "IWinApp.h"

using namespace Zzz;
using namespace Zzz::Platforms;

IWinApp::IWinApp(function<void(const zSize& size, e_TypeWinAppResize resType)> _resizeWindows) :
	resizeWindows{ _resizeWindows },
	winSize{ 0, 0 }
{
}

IWinApp::~IWinApp()
{
}
