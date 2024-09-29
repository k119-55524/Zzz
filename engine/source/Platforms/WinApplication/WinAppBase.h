#pragma once

#include "../../helpers/zResult.h"

namespace Zzz
{
	class zEngine;
}

namespace Zzz::Platforms
{
	class WinAppBase
	{
		//friend class DirectX12API;

	public:
		//virtual const zf32_2D* GetWinSize() = 0;

	//protected:
		virtual zResult Initialize(const s_zWinCreateSetting* data) = 0;
		//zf32_2D winSize;
	};
}