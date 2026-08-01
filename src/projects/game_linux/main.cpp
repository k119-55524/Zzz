#include "main.h"
#include <common/Macroses.h>

using namespace zzz::engine;

// Linux
int main(int argc, char* argv[])
{
	int exitCode = 0;

	{
		DOut("[Linux OS]. Игра запущена.");

		try
		{
			Engine engine("GameLinux_ZzzEngine");
			auto res = engine.Run();
			if (!res)
			{
				DOutError("[Linux OS]. Ошибка выполнения игры: {}.", res.error());
				exitCode = -1;
			}
			else
			{
				DOut("[Linux OS]. Игра завершена успешно.");
			}
		}
		catch (const std::exception& e)
		{
			DOutException("[Linux OS]. Исключение при запуске игры: {}.", e.what());
			exitCode = -1;
		}
		catch (...)
		{
			DOutException("[Linux OS]. Неизвестное исключение при запуске игры.");
			exitCode = -1;
		}
	}

	return exitCode;
}