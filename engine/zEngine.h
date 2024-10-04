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

	private:
		FactoryInit factoryInit;
		FactoryPlatform factoryPlatform;

		mutex initMutex;
		e_InitState initState;

		shared_ptr<Platform> platform;
		shared_ptr<IMainAppLoop> mainLoop;
	};
}