#include "main.h"
#include <common/macroses.h>

using namespace zzz::engine;

// Linux
int main(int argc, char* argv[])
{
	int exitCode = 0;

	{
		DOut("[Linux OS]. Game started.");

		try
		{
			extern void RegisterAllScripts();
			RegisterAllScripts();

			Engine engine("GameLinux_ZzzEngine");
			auto res = engine.Run();
			if (!res)
			{
				DOutError("[Linux OS]. Game runtime error: {}.", res.error());
				exitCode = -1;
			}
			else
			{
				DOut("[Linux OS]. Game exited successfully.");
			}
		}
		catch (const std::exception& e)
		{
			DOutException("[Linux OS]. Game started exception: {}.", e.what());
			exitCode = -1;
		}
		catch (...)
		{
			DOutException("[Linux OS]. Game started unknown exception.");
			exitCode = -1;
		}
	}

	return exitCode;
}