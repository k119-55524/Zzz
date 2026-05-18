#include "pch.h"

#include "modules/main.h"

import engine;
import logger;

using namespace zzz::engine;

int main(int argc, char* argv[])
{
	DOut(L"Game started.");

	Engine engine;

	engine.Initialize();

	return 0;
}