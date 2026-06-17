#pragma once


#include "MainLoop_Common.h"

namespace zzz::engine
{
	class MainLoop_iOS final : public MainLoopBase
	{
		Z_NO_COPY_MOVE(MainLoop_iOS);

	public:
		MainLoop_iOS() = delete;
		MainLoop_iOS(const Platform& platform);
		virtual ~MainLoop_iOS() = default;

		void Run() override;
	};
}
