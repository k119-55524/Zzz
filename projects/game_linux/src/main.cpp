#include "pch.h"

#include "modules/main.h"

import engine;
import logger;

using namespace zzz::engine;

// Linux
int main(int argc, char* argv[])
{
	DOut(L">>>>> [Linux OS]. Game started.");

	Engine engine;
	auto res = engine.Initialize();
	if (res)
	{
		
	}
	else
	{
		DOut(L">>>>> [Linux OS]. Game started error: {}.", res.error());
		return -1;
	}

	return 0;
}