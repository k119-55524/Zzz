#pragma once

#include <mutex>
#include <string>
#include <expected>

#include "foundation/enums.h"

using namespace zzz;

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
