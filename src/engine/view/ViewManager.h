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
		[[nodiscard]] inline std::shared_ptr<View> GetPrimaryView() const noexcept { return m_PrimaryView; }

		/**
		 * @brief Создаёт первичное (основное) окно приложения из ресурса пакета и пользовательских настроек ( user.dat ).
		 */
		void CreatePrimaryView();

		/**
		 * @brief Создаёт дочернее окно по его GUID из ресурсов пакета и настроек user.dat.
		 * Если окно открывается впервые, его положение автоматически вычисляется по центру экрана ( CenterOnWorkArea ).
		 * @param viewGuid Идентификатор ресурса ChildView.
		 */
		void CreateChildView(const Guid& viewGuid);

		/**
		 * @brief Создаёт независимое окно по его GUID из ресурсов пакета и настроек user.dat.
		 * Если окно открывается впервые, его положение автоматически вычисляется по центру экрана ( CenterOnWorkArea ).
		 * @param viewGuid Идентификатор ресурса IndependentView.
		 */
		void CreateIndependentView(const Guid& viewGuid);

#if Z_EDITOR
		std::shared_ptr<View> CreateView(void* data);
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
		ThreadPool m_ThreadsUpdate;

		std::function<void()> OnAllViewsClosed;
		void OnWindowClose(View& view);
		[[nodiscard]] std::vector<std::shared_ptr<ViewScript>> CreateViewScripts(const std::vector<Guid>& scriptGuids) const;
		std::shared_ptr<View> CreateViewInstance(const Guid& viewGuid, const ViewPlatformData& platformData, const std::vector<Guid>& uiScriptGuids, bool isFirstTime, const View* parentView = nullptr);
	};
}
