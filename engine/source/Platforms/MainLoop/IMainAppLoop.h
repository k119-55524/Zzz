#pragma once

#include "../GAPI/IGAPI.h"
#include "../WinApplication/IWinApp.h"

//using namespace Zzz;

namespace Zzz::Platforms
{
	class IMainAppLoop
	{
	public:
		IMainAppLoop() = delete;
		IMainAppLoop(IMainAppLoop&) = delete;
		IMainAppLoop(IMainAppLoop&&) = delete;

		IMainAppLoop(unique_ptr<IWinApp> _win, unique_ptr<IGAPI> _gapi);

		virtual ~IMainAppLoop() = 0;

		virtual void Run() = 0;
		zResult Initialize(const DataEngineInitialization& initData);

	protected:
		unique_ptr<IWinApp> win;
		unique_ptr<IGAPI> gapi;
	};
}
