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
	public:
		WinAppBase();
		WinAppBase(WinAppBase&) = delete;
		WinAppBase(WinAppBase&&) = delete;

		//WinAppBase(shared_ptr<Platform>& platform);
		//virtual ~WinAppBase() = 0;

		zResult Initialize(const s_zEngineInit::WinAppSettings* data);

	protected:
		virtual zResult Init(const s_zEngineInit::WinAppSettings* data) = 0;


		//zf32_2D winSize;

	private:
		mutex initMutex;
		e_InitState initState;
	};
}