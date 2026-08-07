#pragma once

#include "engine/EngineIncludes.h"
#include "engine/utils/Fwd.h"

namespace zzz::engine
{
	using namespace zzz::core;

	class ViewManager final
	{
		Z_NO_MOVE(ViewManager);

	public:
		ViewManager() = delete;
		ViewManager(const Platform& platform, std::shared_ptr<IGAPI> gapi, std::function<void()> onAllViewsClosed);
		~ViewManager();

		[[nodiscard]] inline std::shared_ptr<IGAPI> GetGAPI() const noexcept { return m_GAPI; }

		[[nodiscard]] std::expected<std::shared_ptr<View>, std::string> CreateStartView(const PackageManager& packageManager, const UserSettingsManager& userSettingsManager);
		[[nodiscard]] std::expected <std::shared_ptr<View>, std::string> CreateView(const ViewData& viewData, const std::vector<std::shared_ptr<ViewScript>>& scripts);
#if Z_EDITOR
		[[nodiscard]] std::expected <std::shared_ptr<View>, std::string> CreateView(void* data);
		void RemoveView(View* view);
#endif

		void Update(const Time& time);

	private:
		const Platform& m_Platform;
		std::shared_ptr<IGAPI> m_GAPI;
		std::list<std::shared_ptr<View>> m_Views;

		std::function<void()> OnAllViewsClosed;
		void OnWindowClose(View& view);
	};
}
