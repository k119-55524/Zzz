#pragma once

#include "source/Structs.h"
#include "source/UserGameSettings.h"
#include "source/Platforms/IO/IIO.h"
#include "source/Platforms/Platform.h"
#include "source/Factories/FactoryInit.h"
#include "source/Factories/FactoryPlatform.h"
#include "source/SceneEntities/SceneManager.h"
#include "source/Platforms/MainLoop/IMainAppLoop.h"

using namespace Zzz::Core;
using namespace Zzz::Platforms;

namespace Zzz
{
	class zEngine
	{
	public:
		zEngine();
		~zEngine();

		zResult Initialize();
		zResult Run();

	private:
		FactoryInit factoryInit;
		FactoryPlatform factoryPlatform;

		e_InitState initState;
		mutex initMutex;
		mutex runMutex;

		unique_ptr<Platform> platform;
		shared_ptr<IIO> platformIO;
		shared_ptr<UserGameSettings> userGS;
		shared_ptr<IWinApp> appWin;
		unique_ptr<IGAPI> gAPI;
		shared_ptr<IMainAppLoop> mainLoop;

		bool isAppPaused;
		void Reset(e_InitState state = e_InitState::eInitNot);

		void OnUpdateSystem();
		void OnResizeWindow(const zSize& size, e_TypeWinAppResize resizeType);

		SceneManager sceneManager;
	};
}