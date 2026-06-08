#pragma once

#if defined(Z_MACOS)

#include "IMainLoop.h"

namespace zzz::engine
{
	class MainLoop_MacOS final : public IMainLoop
	{
		Z_NO_COPY_MOVE(MainLoop_MacOS);

	public:
		MainLoop_MacOS() = delete;
		MainLoop_MacOS(const std::shared_ptr<IPlatform> platform);
		virtual ~MainLoop_MacOS() = default;

		void Run() override;
	};
}
#endif // defined(Z_MACOS)
