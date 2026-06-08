#pragma once

#if defined(Z_IOS)

#include "IMainLoop.h"

namespace zzz::engine
{
	class MainLoop_iOS final : public IMainLoop
	{
		Z_NO_COPY_MOVE(MainLoop_iOS);

	public:
		MainLoop_iOS() = delete;
		MainLoop_iOS(const std::shared_ptr<IPlatform> platform);
		virtual ~MainLoop_iOS() = default;

		void Run() override;
	};
}
#endif // defined(Z_IOS)
