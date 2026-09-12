#include "android_main.h"

using namespace zzz;
using namespace zzz::logger;
using namespace zzz::engine;

extern "C"
__attribute__((visibility("default")))
__attribute__((used))
void android_main(struct android_app* app)
{
	DOut("[Android]. Игра запущена.");

	std::unique_ptr<Engine> engine;

	try
	{
		auto platformData = std::shared_ptr<android_app>(app, [](android_app*) {});
		engine = safe_make_unique<Engine>(platformData);
		// Запускаем движок. Он сам будет крутить цикл внутри MainLoopAndroid
		auto runRes = engine->Run();
		if (!runRes)
		{
			DOut("[Android]. Ошибка запуска движка: {}.", runRes.error());
		}
	}
	catch (const std::exception& e)
	{
		DOut("[Android]. Исключение при запуске игры: {}.", e.what());
		return;
	}
	catch (...)
	{
		DOut("[Android]. Неизвестное исключение при запуске игры.");
		return;
	}

	DOut("[Android]. Завершение игры.");
}
