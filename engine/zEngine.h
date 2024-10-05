#pragma once

#include "source/Structs.h"
#include "source/Platforms/Platform.h"
#include "source/Factories/FactoryInit.h"
#include "source/Factories/FactoryPlatform.h"
#include "source/Platforms/MainLoop/IMainAppLoop.h"

using namespace Zzz::Platforms;

namespace Zzz
{
	class zEngine
	{
	public:
		zEngine();

		zResult Initialize(const DataEngineInitialization initData);
		zResult Run();

	private:
		FactoryInit factoryInit;
		FactoryPlatform factoryPlatform;

		e_InitState initState;
		mutex initMutex;
		mutex runMutex;

		shared_ptr<Platform> platform;
		shared_ptr<IWinApp> appWin;
		unique_ptr<IGAPI> gAPI;
		shared_ptr<IMainAppLoop> mainLoop;

		bool isAppPaused;
		void Reset(e_InitState state = e_InitState::eInitNot);

		void OnUpdateSystem();
		void OnResizeWindow(const zSize& size, e_TypeWinAppResize resizeType);
	};
}