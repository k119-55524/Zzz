#include <android/looper.h>

#include "MainLoop_Android.h"
#include "../Platform.h"

using namespace zzz::engine;

namespace zzz::engine {
    void ProcessAndroidInput(struct android_app* app);
}

MainLoop_Android::MainLoop_Android(const Platform& platform) :
	MainLoopBase(platform)
{
}

void MainLoop_Android::Run()
{
	isRunning.store(true);

	android_app* app = m_Platform->GetNativeData().get();
	while (isRunning.load())
	{
		int ident;
		int events;
		struct android_poll_source* source;

		// 0 - если событий нет)
		while ((ident = ALooper_pollOnce(0, nullptr, &events, (void**)&source)) >= 0)
		{
			if (source != nullptr)
			{
				source->process(app, source);
			}

			if (app->destroyRequested != 0)
			{
				DOut("[Android Loop]. Destroy requested.");
				Stop();
				break;
			}
		}

		ProcessAndroidInput(app);

		if (!isRunning.load())
			break;

		onUpdateSystem();
	}
}

