#pragma once


#include "MainLoop_Common.h"

namespace zzz::engine
{
	class MainLoop_MacOS final : public MainLoopBase
	{
		Z_NO_COPY_MOVE(MainLoop_MacOS);

	public:
		MainLoop_MacOS() = delete;
		MainLoop_MacOS(const Platform& platform);
		virtual ~MainLoop_MacOS() = default;

		void Run() override;
	};
}
