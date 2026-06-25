#pragma once

#include <list>
#include <memory>
#include <functional>

namespace zzz::engine
{
	class Platform;
	class View;

	class ViewManager final
	{
		Z_NO_MOVE(ViewManager);

	public:
		ViewManager() = delete;
		ViewManager(const Platform& platform, std::function<void()> onAllViewsClosed);
		~ViewManager();

		View* CreateView();
#if Z_EDITOR
		View* CreateView(void* data);
#endif
		void DestroyView(View* view);

	private:
		const Platform& m_Platform;
		std::list<std::shared_ptr<View>> m_Views;

		std::function<void()> OnAllViewsClosed;
		void HandleWindowClose(View& view);
	};
}
