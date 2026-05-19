#include "pch.h"

#include "modules/main.h"

import engine;
import logger;

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
	}
}