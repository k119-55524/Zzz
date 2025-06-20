
#include "pch.h"
export module engine;

import error;

export namespace zzz {
	export class engine {
	public:
		unique_ptr<error> Init();

	private:

	};
}

using namespace zzz;

unique_ptr<error> engine::Init()
{
	auto err = make_unique<error>();
	return err;
}
