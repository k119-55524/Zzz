#if defined(Z_ANDROID)

#include <android/looper.h>

#include "MainLoop_Android.h"
#include "../platforms/PlatformAndroid.h"

using namespace zzz::engine;

MainLoop_Android::MainLoop_Android(const std::shared_ptr<IPlatform> platform) :
	IMainLoop(platform)
{
}

void MainLoop_Android::Run()
{
	isRunning.store(true);

	android_app* app = m_Platform->GetPlatformData().get();
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

		PlatformAndroid::ProcessInput(app);

		if (!isRunning.load())
			break;

		onUpdateSystem();
	}
}

#endif // defined(Z_ANDROID)
