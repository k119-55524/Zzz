#include "pch.h"
export module scene;

import resourcesManager;

export namespace zzz
{
	export class scene final
	{
	public:
		scene();
		scene(const scene&) = delete;
		scene(scene&&) = delete;
		scene& operator=(const scene&) = delete;
		scene& operator=(scene&&) = delete;

		~scene();

	};

	export scene::scene()
	{

	}

	export scene::~scene()
	{
	}
}