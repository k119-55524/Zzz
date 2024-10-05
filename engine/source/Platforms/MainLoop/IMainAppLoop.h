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

		IMainAppLoop(function<void()> _updateSystem);

		virtual ~IMainAppLoop() = 0;

		virtual void Run() = 0;

	protected:
		function<void()> updateSystem;
	};
}
