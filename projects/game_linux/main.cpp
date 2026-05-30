#include "main.h"

using namespace zzz::engine;

// Linux
int main(int argc, char* argv[])
{
	DOut(">>>>> [Linux OS]. Game started.");

	try
	{
		Engine engine("GameLinux_ZzzEngine");
		auto res = engine.Initialize();
		if (res)
		{
		
		}
		else
		{
			DOut(">>>>> [Linux OS]. Game started error: {}.", res.error());
			return -1;
		}
	}
	catch (const std::exception& e)
	{
		DOut(">>>>> [Linux OS]. Game started exception: {}.", e.what());
		return -1;
	}
	catch (...)
	{
		DOut(">>>>> [Linux OS]. Game started unknown exception.");
		return -1;
	}

	return 0;
}