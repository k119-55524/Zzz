module;

#include <expected>
#include "foundation/enums.h"

export module engine;

using namespace zzz;

export namespace zzz::engine
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