#pragma once

#include "../../Structs.h"
#include "../../helpers/zResult.h"

namespace Zzz
{
	class zEngine;
}

namespace Zzz::Platforms
{
	class IWinApp
	{
	public:
		IWinApp() = default;
		IWinApp(IWinApp&) = delete;
		IWinApp(IWinApp&&) = delete;
		virtual ~IWinApp() = 0;

		virtual zResult Initialize(const DataEngineInitialization& data) = 0;

	protected:
		//zf32_2D winSize;
	};
}