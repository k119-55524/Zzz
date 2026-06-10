#pragma once

#include <list>
#include <memory>

#include "../../../header.h"
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
		ViewManager(std::shared_ptr<Platform> platform);
		~ViewManager();

		void CreateView();

		inline bool HasViews() const noexcept { return !m_Views.empty(); }

		std::function<void()> onAllViewsClosed;

	private:
		std::shared_ptr<Platform> m_Platform;
		std::list<std::shared_ptr<View>> m_Views;
	};
}
