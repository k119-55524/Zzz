#pragma once

#if defined(Z_LINUX)

#include "IMainLoop.h"

namespace zzz::engine
{
	class MainLoop_Linux final : public IMainLoop
	{
		Z_NO_COPY_MOVE(MainLoop_Linux);

	public:
		MainLoop_Linux() = default;
		virtual ~MainLoop_Linux() = default;

		void Run() override;
	};
}
#endif // defined(Z_LINUX)