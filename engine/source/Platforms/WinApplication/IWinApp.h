#pragma once

#include "../../Structs.h"
#include "../../helpers/zResult.h"

namespace Zzz
{
	class zEngine;

	enum e_TypeWinAppResize : zU32
	{
		eShow,
		eHide,
		eResize
	};
}

namespace Zzz::Platforms
{
	class IWinApp
	{
	public:
		IWinApp() = delete;
		IWinApp(IWinApp&) = delete;
		IWinApp(IWinApp&&) = delete;

		IWinApp(function<void(const zSize& size, e_TypeWinAppResize resType)> _resizeWindows);
		virtual ~IWinApp() = 0;

		virtual zResult Initialize(const DataEngineInitialization& data) = 0;
		//virtual void Show() = 0;

	protected:
		zSize winSize;
		function<void(const zSize& size, e_TypeWinAppResize resType)> resizeWindows;
	};
}