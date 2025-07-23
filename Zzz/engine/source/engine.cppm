
#include "pch.h"
export module engine;

import zzz;
using namespace zzz;
using namespace zzz::error;

export namespace zzz
{
	export class engine
	{
	public:
		result<zResult> initialize();
		result<zResult> go();

	private:

	};
}

result<zResult> engine::initialize()
{
	//return Unexpected(zResult::failure);
	//return Unexpected(zResult::failure, "Error");
	return zResult::success;
}

result<zResult> engine::go()
{
	return zResult::success;
}
