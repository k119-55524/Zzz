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
		ViewManager(std::shared_ptr<Platform> platform, std::function<void()> onAllViewsClosed);
		~ViewManager();

		void CreateView();

	private:
		std::shared_ptr<Platform> m_Platform;
		std::list<std::shared_ptr<View>> m_Views;

		std::function<void()> OnAllViewsClosed;
	};
}
