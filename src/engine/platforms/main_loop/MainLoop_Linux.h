#pragma once


#include "MainLoop_Common.h"

struct wl_display;

namespace zzz::engine
{
	class MainLoop_Linux final : public MainLoopBase
	{
		Z_NO_COPY_MOVE(MainLoop_Linux);

	public:
		MainLoop_Linux() = delete;
		MainLoop_Linux(const Platform& platform, std::function<void()> onUpdate);
		virtual ~MainLoop_Linux() = default;

		void Run() override;

	private:
		wl_display* m_Display;
	};
}
