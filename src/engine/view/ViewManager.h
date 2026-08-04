#pragma once

#include <list>
#include <memory>
#include <functional>

#include <vector>

namespace zzz::script
{
	class ViewScript;
}

#include <expected>
#include <string>

namespace zzz::core
{
	class ViewData;
}

namespace zzz::engine
{
	class Platform;
	class View;
	class Time;
	class PackageManager;

	class ViewManager final
	{
		Z_NO_MOVE(ViewManager);

	public:
		ViewManager() = delete;
		ViewManager(const Platform& platform, std::function<void()> onAllViewsClosed);
		~ViewManager();

		[[nodiscard]] std::expected<std::shared_ptr<View>, std::string> InitializeFromPackage(const PackageManager& packageManager);
		[[nodiscard]] std::expected <std::shared_ptr<View>, std::string> CreateView(const zzz::core::ViewData& viewData, const std::vector<std::shared_ptr<zzz::script::ViewScript>>& scripts);
#if Z_EDITOR
		[[nodiscard]] std::expected <std::shared_ptr<View>, std::string> CreateView(void* data);
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
