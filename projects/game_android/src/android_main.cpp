#include "android_main.h"

using namespace zzz::logger;
using namespace zzz::engine;

void android_main(struct android_app* app)
{
	DOut(L"[Android]. Game started.");

	Engine engine;

	auto res = engine.Initialize();

	if (!res)
	{
		DOut(
			L"[Android]. Engine init error: {}.",
			res.error());

		return;
	}

	while (true)
	{
		int ident;
		int events;
		struct android_poll_source* source;

		while ((ident = ALooper_pollAll(0, nullptr, &events, (void**)&source)) >= 0)
		{
			if (source != nullptr)
			{
				source->process(app, source);
			}

			if (app->destroyRequested != 0)
			{
				DOut(L"[Android]. Game exiting.");
				return;
			}
		}

		// Тут будет логика кадра (Update/Render)
	}
}
