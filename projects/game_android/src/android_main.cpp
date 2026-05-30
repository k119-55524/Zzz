#include "android_main.h"

using namespace zzz;
using namespace zzz::logger;
using namespace zzz::engine;

extern "C"
__attribute__((visibility("default")))
__attribute__((used))
void android_main(struct android_app* app)
{
	DOut("[Android]. Game started.");

	try
	{
		auto platformData = std::shared_ptr<android_app>(app, [](android_app*) {});
		Engine engine("GameAndroid_ZzzEngine", platformData);
		auto res = engine.Initialize();
		if (!res)
		{
			DOut("[Android]. Engine init error: {}.", res.error());
			return;
		}
	}
	catch (const std::exception& e)
	{
		DOut("[Android]. Game started exception: {}.", e.what());
		return;
	}
	catch (...)
	{
		DOut("[Android]. Game started unknown exception.");
		return;
	}

	while (true)
	{
		int ident;
		int events;
		struct android_poll_source* source;

		while ((ident = ALooper_pollOnce(0, nullptr, &events, (void**)&source)) >= 0)
		{
			if (source != nullptr)
			{
				source->process(app, source);
			}

			if (app->destroyRequested != 0)
			{
				DOut("[Android]. Game exiting.");
				return;
			}
		}

		// Тут будет логика кадра (Update/Render)
	}
}
