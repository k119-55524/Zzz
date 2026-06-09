#pragma once

#if defined(Z_WINDOWS)

#include "IMainLoop.h"

namespace zzz::engine
{
	class MainLoop_MSWin final : public IMainLoop
	{
		Z_NO_COPY_MOVE(MainLoop_MSWin);

	public:
		MainLoop_MSWin() = delete;
		MainLoop_MSWin(const std::shared_ptr<Platform> platform);
		virtual ~MainLoop_MSWin() = default;

		void Run() override;
	};
}
#endif // defined(Z_WINDOWS)