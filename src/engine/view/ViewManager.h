#pragma once

#include "engine/view/View.h"
#include "engine/gapi/IGAPI.h"
#include "engine/EngineIncludes.h"
#include "engine/platforms/Platform.h"
#include "engine/package/PackageManager.h"
#include "engine/package/UserSettingsManager.h"

using namespace zzz::core;
using namespace zzz::templates;

namespace zzz::engine
{
	class ViewManager final
	{
		Z_NO_MOVE(ViewManager);

	public:
		ViewManager() = delete;
		ViewManager(const Platform& platform, std::shared_ptr<IGAPI> gapi, std::shared_ptr<ScriptFactory> scriptFactory, std::shared_ptr<PackageManager> packageManager, std::shared_ptr<UserSettingsManager> userSettingsManager, std::function<void()> onAllViewsClosed);
		~ViewManager();

		[[nodiscard]] inline std::shared_ptr<IGAPI> GetGAPI() const noexcept { return m_GAPI; }

		/**
		 * @brief Создаёт первичное (основное) окно приложения из стартового ресурса пакета и пользовательских настроек окна.
		 */
		[[nodiscard]] std::expected<std::shared_ptr<View>, std::string> CreatePrimaryView();
		[[nodiscard]] std::expected<std::shared_ptr<View>, std::string> CreateChildView(const Guid& viewGuid);
		[[nodiscard]] std::expected<std::shared_ptr<View>, std::string> CreateIndependentView(const Guid& viewGuid);

#if Z_EDITOR
		[[nodiscard]] std::expected <std::shared_ptr<View>, std::string> CreateView(void* data);
		void RemoveView(View* view);
#endif

		void Update(const Time& time);

	private:
		const Platform& m_Platform;
		std::shared_ptr<IGAPI> m_GAPI;
		std::shared_ptr<ScriptFactory> m_ScriptFactory;
		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<UserSettingsManager> m_UserSettingsManager;

		std::shared_ptr<View> m_PrimaryView;
		std::vector<std::shared_ptr<View>> m_ChildViews;
		std::vector<std::shared_ptr<View>> m_IndependentViews;
		std::list<std::shared_ptr<View>> m_Views;
		ThreadPool m_ThreadsUpdate;

		std::function<void()> OnAllViewsClosed;
		void OnWindowClose(View& view);
		[[nodiscard]] std::vector<std::shared_ptr<ViewScript>> CreateViewScripts(const std::vector<Guid>& scriptGuids) const;
	};
}
