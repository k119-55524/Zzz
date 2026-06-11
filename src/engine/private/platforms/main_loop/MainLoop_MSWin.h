#pragma once


#include "MainLoop_Common.h"

namespace zzz::engine
{
	class MainLoop_MSWin final : public MainLoopBase
	{
		Z_NO_COPY_MOVE(MainLoop_MSWin);

	public:
		MainLoop_MSWin() = delete;
		MainLoop_MSWin(const std::shared_ptr<Platform> platform, std::function<void()> onUpdate);
		virtual ~MainLoop_MSWin() = default;

		void Run() override;
	};
}
