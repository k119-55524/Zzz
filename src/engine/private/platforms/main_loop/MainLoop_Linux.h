#pragma once

#if defined(Z_LINUX)

#include "IMainLoop.h"

struct wl_display;

namespace zzz::engine
{
	class MainLoop_Linux final : public IMainLoop
	{
		Z_NO_COPY_MOVE(MainLoop_Linux);

	public:
		MainLoop_Linux() = delete;
		MainLoop_Linux(const std::shared_ptr<IPlatform> platform);
		virtual ~MainLoop_Linux() = default;

		void Run() override;

	private:
		wl_display* m_Display;
	};
}
#endif // defined(Z_LINUX)