#pragma once

using namespace zzz;

#include "header.h"

namespace zzz::engine
{
	class Engine
	{
	public:
		Engine();

		[[nodiscard]] std::expected<void, std::wstring> Initialize();

	private:
		std::mutex initMutex;
		eInitState initState;
	};
}
