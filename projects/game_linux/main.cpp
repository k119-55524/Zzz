#include "main.h"

using namespace zzz::engine;

// Linux
int main(int argc, char* argv[])
{
	DOut(">>>>> [Linux OS]. Game started.");

	Engine engine;
	auto res = engine.Initialize();
	if (res)
	{
		
	}
	else
	{
		DOut(">>>>> [Linux OS]. Game started error: {}.", res.error());
		return -1;
	}

	return 0;
}