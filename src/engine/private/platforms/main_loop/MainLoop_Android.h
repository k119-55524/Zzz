#pragma once


#include "MainLoop_Common.h"

namespace zzz::engine
{
	class MainLoop_Android final : public MainLoopBase
	{
		Z_NO_COPY_MOVE(MainLoop_Android);

	public:
		MainLoop_Android() = delete;
		MainLoop_Android(const std::shared_ptr<Platform> platform);
		virtual ~MainLoop_Android() = default;

		void Run() override;
	};
}
