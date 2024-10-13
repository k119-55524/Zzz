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

#include "../../UserGameSettings.h"

using namespace Zzz;

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

		virtual void Initialize(const shared_ptr<UserGameSettings> userGS) = 0;
		inline const zSize& GetWinSize() const noexcept { return winSize; };

	protected:
		zSize winSize;
		function<void(const zSize& size, e_TypeWinAppResize resType)> resizeWindows;
	};
}