#pragma once

#include "core/Core.h"
#include "engine/gapi/GAPI.h"
#include "engine/view/View.h"
#include "engine/platforms/Platform.h"
#include "engine/package/PackageManager.h"
#include "engine/package/UserSettingsManager.h"
#include "engine/tasks/TaskDispatcher.h"

using namespace zzz::core;
using namespace zzz::templates;

namespace zzz::engine
{
	class ViewManager final
	{
		Z_NO_MOVE(ViewManager);

	public:
		ViewManager() = delete;
		ViewManager(
			TaskDispatcher& taskDispatcher,
			const Platform& platform,
			std::shared_ptr<GAPI> gapi,
			std::shared_ptr<ScriptFactory> scriptFactory,
			std::shared_ptr<PackageManager> packageManager,
			std::shared_ptr<UserSettingsManager> userSettingsManager,
			std::shared_ptr<SceneManager> sceneManager,
			std::function<void()> onAllViewsClosed
		);
		~ViewManager();

		[[nodiscard]] inline std::shared_ptr<GAPI> GetGAPI() const noexcept { return m_GAPI; }
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
		TaskDispatcher& m_TaskDispatcher;
		const Platform& m_Platform;
		std::shared_ptr<GAPI> m_GAPI;
		std::shared_ptr<ScriptFactory> m_ScriptFactory;
		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<UserSettingsManager> m_UserSettingsManager;
		std::shared_ptr<SceneManager> m_SceneManager;

		std::shared_ptr<View> m_PrimaryView;
		std::vector<std::shared_ptr<View>> m_ChildViews;
		std::vector<std::shared_ptr<View>> m_IndependentViews;

		std::function<void()> OnAllViewsClosed;
		void OnWindowClose(View& view);
		std::shared_ptr<View> CreateViewInstance(const ViewConfigData& viewData, ePackage viewType, const View* parentView = nullptr);
		void SetInitialSceneAsync(std::weak_ptr<View> viewWeak, const Guid& sceneGuid);
	};
}
