#pragma once

#include <list>
#include <memory>
#include <functional>

#include <vector>

namespace zzz::script
{
	class ViewScript;
}

namespace zzz::engine
{
	class Platform;
	class View;
	class Time;

	class ViewManager final
	{
		Z_NO_MOVE(ViewManager);

	public:
		ViewManager() = delete;
		ViewManager(const Platform& platform, std::function<void()> onAllViewsClosed);
		~ViewManager();

		View* CreateView(const std::string_view viewName, const std::vector<std::shared_ptr<zzz::script::ViewScript>>& scripts);
#if Z_EDITOR
		View* CreateView(void* data);
		void RemoveView(View* view);
#endif

		void Update(const zzz::engine::Time& time);

	private:
		const Platform& m_Platform;
		std::list<std::shared_ptr<View>> m_Views;

		std::function<void()> OnAllViewsClosed;
		void OnWindowClose(View& view);
	};
}
