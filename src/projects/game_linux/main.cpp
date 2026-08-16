#include "main.h"
#include <core/Core.h>

using namespace zzz::engine;

// Linux
int main(int argc, char* argv[])
{
	int exitCode = 0;

	{
		DOut("[Main::main (Linux)] - Игра запущена.");

		try
		{
			Engine engine("GameLinux_ZzzEngine");
			auto res = engine.Run();
			if (!res)
			{
				DOutError("[Main::main (Linux)] - Ошибка выполнения игры: {}.", res.error());
				exitCode = -1;
			}
			else
			{
				DOut("[Main::main (Linux)] - Игра завершена успешно.");
			}
		}
		catch (const std::exception& e)
		{
			DOutException("[Main::main (Linux)] - Исключение при запуске игры: {}.", e.what());
			exitCode = -1;
		}
		catch (...)
		{
			DOutException("[Main::main (Linux)] - Неизвестное исключение при запуске игры.");
			exitCode = -1;
		}
	}

	return exitCode;
}
