#include "core/utils/Defines.h"

#if defined(Z_ANDROID)

#include <android/looper.h>
#include "MainLoopAndroid.h"
#include "../Platform.h"

Z_SET_LOG_CATEGORY(::zzz::core::LogEngine);

using namespace zzz::engine;

namespace zzz::engine {
    void ProcessAndroidInput(struct android_app* app);
}

MainLoopAndroid::MainLoopAndroid(const Platform& platform, std::function<void()> onUpdate) :
	MainLoopBase(platform, std::move(onUpdate))
{
}

void MainLoopAndroid::Run()
{
	isRunning.store(true);

	android_app* app = m_Platform.GetNativeData().get();
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
				DOut("[Android Loop]. Запрошено уничтожение.");
				Stop();
				break;
			}
		}

		ProcessAndroidInput(app);

		if (!isRunning.load())
			break;

		OnUpdate();
	}
}

#endif // defined(Z_ANDROID)

