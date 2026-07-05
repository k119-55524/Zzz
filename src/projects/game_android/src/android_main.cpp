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

	std::unique_ptr<Engine> engine;

	try
	{
		extern void RegisterAllScripts();
		RegisterAllScripts();

		auto platformData = std::shared_ptr<android_app>(app, [](android_app*) {});
		engine = safe_make_unique<Engine>("GameAndroid_ZzzEngine", platformData);
		// Запускаем движок. Он сам будет крутить цикл внутри MainLoop_Android
		auto runRes = engine->Run();
		if (!runRes)
		{
			DOut("[Android]. Engine run error: {}.", runRes.error());
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

	DOut("[Android]. Game exiting.");
}
