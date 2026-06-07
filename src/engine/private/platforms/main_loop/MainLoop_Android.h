#pragma once

#if defined(Z_ANDROID)

#include "IMainLoop.h"

namespace zzz::engine
{
	class MainLoop_Android final : public IMainLoop
	{
		Z_NO_COPY_MOVE(MainLoop_Android);

	public:
		MainLoop_Android() = delete;
		MainLoop_Android(const std::shared_ptr<IPlatform> platform);
		virtual ~MainLoop_Android() = default;

		void Run() override;
	};
}
#endif // defined(Z_ANDROID)
